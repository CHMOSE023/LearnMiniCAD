#include "GripEditor.h"
#include "Core/Entity/LineEntity.hpp"

using namespace DirectX;

namespace MiniCAD
{
    bool GripEditor::OnMouseDown(const InputEvent& e)
    {
        if (e.Button != MouseButton::Left) return false;

        XMFLOAT2 sp((float)e.MouseX, (float)e.MouseY);
        int idx = m_grips->HitTest(sp, m_scene->GetCamera());

        if (idx < 0) return false;   // 没命中，让 Picking 处理

        m_dragging = true;
        m_activeIdx = idx;
        return true;                 // 消费事件
    }

    bool GripEditor::OnMouseMove(const InputEvent& e)
    {
        XMFLOAT2 sp((float)e.MouseX, (float)e.MouseY);
        const Camera* cam = m_scene->GetCamera();

        // 更新 hover（无论是否拖拽）
        m_hoveredIdx = m_grips->HitTest(sp, cam);

        if (!m_dragging) return false;
         

        auto worldPos =  cam->ScreenToWorld(sp.x,sp.y);     
        ApplyDrag(worldPos);
        return true;
    }

    bool GripEditor::OnMouseUp(const InputEvent& e)
    {
        if (e.Button != MouseButton::Left || !m_dragging) return false;

        m_dragging = false;
        m_activeIdx = -1; 
         

        return true;
    }

    // ─── 根据夹点类型修改几何 ────────────────────────────────────────────────

    void GripEditor::ApplyDrag(const XMFLOAT3& worldPos)
    {
        const Grip& grip = m_grips->GetGrips()[m_activeIdx];

        Object* obj = m_scene->GetEntity(grip.OwnerID);
        if (!obj) return;

        auto* line = dynamic_cast<LineEntity*>(obj);
        if (!line) return;

        auto seg = line->GetLine();   // 取副本

        switch (grip.GripType)
        {
        case Grip::Type::Start:
            seg.Start = worldPos;
            break;

        case Grip::Type::End:
            seg.End = worldPos;
            break;

        case Grip::Type::Mid:
        {
            // 整体平移：保持线段长度和方向
            XMFLOAT2 oldMid = { (seg.Start.x + seg.End.x) * 0.5f, (seg.Start.y + seg.End.y) * 0.5f };
            float dx = worldPos.x - oldMid.x;
            float dy = worldPos.y - oldMid.y;

            seg.Start = { seg.Start.x + dx, seg.Start.y + dy , 0};
            seg.End   = { seg.End.x + dx, seg.End.y + dy , 0};

            break;
        }
        }

        line->SetLine(seg);

        // 实时更新夹点位置（拖拽中不需要全量 Rebuild，只改这一个）
        // 注意：Mid 夹点需要同时更新 Start/End 对应的夹点
        // 简单起见：直接全量 Rebuild（选集没变，开销可接受）
        // 如果性能敏感可以只 patch 对应索引
        // m_grips->Rebuild(...)  ← 在外层调用
    }
}