#include "Picking.h"
#include "App/Scene/Scene.h"
#include "Core/Object/Object.hpp"
#include "Render/Viewport/Camera.h"
#include <cmath>
#include <windows.h>

namespace MiniCAD
{  
	// ─── 内部工具函数 ──────────────────────────────────────────────────────────────

    static float PointToSegmentDist(const XMFLOAT2& p, const XMFLOAT2& a, const XMFLOAT2& b)
    {
        float dx    = b.x - a.x, dy = b.y - a.y;
        float lenSq = dx * dx + dy * dy;

        if (lenSq < 1e-8f)  // 线段退化为点
            return std::hypot(p.x - a.x, p.y - a.y);

        // 投影参数 t 夹在 [0,1]
        float t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / lenSq;
        t = std::max(0.f, std::min(1.f, t));

        float cx = a.x + t * dx;
        float cy = a.y + t * dy;
        return std::hypot(p.x - cx, p.y - cy);
    }

	// 拾取最近的
    static Object::ObjectID PickNearest(Scene* scene, const DirectX::XMFLOAT2& mousePt, float thresh = 5)
    {
        Object::ObjectID best     = Object::InvalidID;
        float            bestDist = FLT_MAX;
        auto             camera   = scene->GetCamera();

        scene->ForEachObject([&](const Object& obj)
        { 
            if (const LineEntity* line = dynamic_cast<const LineEntity*>(&obj))
            {
                DirectX::XMFLOAT2 start = camera->WorldToScreen(line->GetLine().Start);
                DirectX::XMFLOAT2 end   = camera->WorldToScreen(line->GetLine().End);
                     
                float d = PointToSegmentDist(mousePt, start, end);
                 
                if (d < thresh && d < bestDist)
                {
                    bestDist = d;
                    best     = obj.GetID();  // 赋值给外层变量
					printf("[Picking] 点选: ID=%llu, Dist=%.2f\n", obj.GetID(), d);
                }
            }
        });

        return best;

    }

	// 计算在矩形框内的对象，返回它们的 ID 集合
    static std::unordered_set<Object::ObjectID> PickInRect(Scene* scene,const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
    {
        float xMin = std::min(a.x, b.x), xMax = std::max(a.x, b.x);
        float yMin = std::min(a.y, b.y), yMax = std::max(a.y, b.y);

        auto camera       = scene->GetCamera();     
        bool fullyContain = (b.x > a.x);    // 从左往右框 → 完全包含模式  从右往左框 → 触碰即选模式
        auto inRect       = [&](const XMFLOAT2& p) { return p.x >= xMin && p.x <= xMax && p.y >= yMin && p.y <= yMax; };

        std::unordered_set<Object::ObjectID> result;

        scene->ForEachObject([&](const Object& obj)
        {
             if (const LineEntity* line = dynamic_cast<const LineEntity*>(&obj))
             {
                 DirectX::XMFLOAT2 start = camera->WorldToScreen(line->GetLine().Start);
                 DirectX::XMFLOAT2 end   = camera->WorldToScreen(line->GetLine().End); 				
            
                 bool hit = fullyContain ? (inRect(start) && inRect(end))   // 两端都在框内
                                         : (inRect(start) || inRect(end));  // 任意一端在框内

                 if (hit)
                 {
                     result.insert(obj.GetID());
					 printf("[Picking] 框选: ID=%llu\n", obj.GetID());
                 }
                 
             }
        });

        return result; 
    }

    // ─── 公开接口 ──────────────────────────────────────────────────────────────   
    void Picking::OnMouseDown(const InputEvent& e)
    {
        if (e.Button != MouseButton::Left) return;

        m_drag = DragState::Pressing;
        m_pressX = e.MouseX;
        m_pressY = e.MouseY;
         
    }

    void Picking::OnMouseMove(const InputEvent& e)
    { 
        if (m_drag == DragState::Pressing)
        {
            int dx = e.MouseX - m_pressX;
            int dy = e.MouseY - m_pressY;

            if (std::abs(dx) > DRAG_THRESH || std::abs(dy) > DRAG_THRESH)
                m_drag = DragState::BoxSelecting;

        }

        if (m_drag != DragState::BoxSelecting)
            UpdateHovered(e);
    }

    void Picking::OnMouseUp(const InputEvent& e)
    {
        if (e.Button != MouseButton::Left) return;

        switch (m_drag)
        {
        case DragState::Pressing:      DoPointPick(e); break;
        case DragState::BoxSelecting:  DoBoxPick(e);   break;
        default: break;
        }

        m_drag = DragState::Idle;
        
    }

    void Picking::OnKeyDown(const InputEvent& e)
    {
        if (e.KeyCode == VK_ESCAPE)
            m_selection.clear();
    }

    // ─── 私有实现 ──────────────────────────────────────────────────────────────

    void Picking::UpdateHovered(const InputEvent& e)
    {
        const Camera* camera = m_scene->GetCamera();

        auto mousePt = DirectX::XMFLOAT2(e.MouseX, e.MouseY);

        Object::ObjectID  hid = PickNearest(m_scene, mousePt, (float)DRAG_THRESH);

       if (m_hovered.size() == 1 && *m_hovered.begin() == hid) return;

        m_hovered.clear();
        if (hid != Object::InvalidID)
            m_hovered.insert(hid);
    }

    void Picking::DoPointPick(const InputEvent& e)
    { 
        const Camera* camera = m_scene->GetCamera();

        bool ctrl    = e.HasModifier(ModifierKey::Ctrl); 

        auto mousePt = DirectX::XMFLOAT2(e.MouseX, e.MouseY);

        Object::ObjectID  clicked = PickNearest(m_scene, mousePt,(float)DRAG_THRESH);

        if (clicked == Object::InvalidID)
        {
            if (!ctrl) m_selection.clear();
            return;
        }

        if (ctrl)
        {
            // Ctrl 点击：切换
            auto it = m_selection.find(clicked);

            if (it != m_selection.end())
                m_selection.erase(it);
            else                         
                m_selection.insert(clicked);
        }
        else
        {
            // 普通点击：替换
            m_selection = { clicked };
        }
    }

    void Picking::DoBoxPick(const InputEvent& e)
    {       
        const Camera* camera = m_scene->GetCamera();
        const bool    ctrl   = e.HasModifier(ModifierKey::Ctrl);

        auto worldA = DirectX::XMFLOAT2(m_pressX, m_pressY);  
        auto worldB = DirectX::XMFLOAT2(e.MouseX, e.MouseY);

        auto boxResult = PickInRect(m_scene, worldA, worldB);

        if (ctrl)
        {
            for (auto id : boxResult)
                m_selection.insert(id);         // unordered_set 不会重复插入
        }
        else
        {
            m_selection = std::move(boxResult); // move 直接替换，不需要先 clear
        }
    }

}  
