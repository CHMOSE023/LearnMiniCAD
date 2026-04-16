#include "GripEditor.h"
#include "Core/Entity/LineEntity.hpp" 
#include "App/Command/DragLineCommand.h" 
#include <memory>
using namespace DirectX;

namespace MiniCAD
{
    bool GripEditor::OnMouseDown(const InputEvent& e)
    {
        if (e.Button != MouseButton::Left) return false;

        XMFLOAT2 sp((float)e.MouseX, (float)e.MouseY);
        int idx = m_gripManager->HitTest(sp, m_scene->GetCamera());
          
        if (idx < 0) return false;   // 没命中，让 Picking 处理 
        
        const auto& grip = m_gripManager->GetGrips()[idx]; 
        auto obj = m_scene->GetEntity(grip.OwnerID);
        if (obj->IsKindOf<LineEntity>())
        {
            m_drag.active = true;                //
            m_drag.id     = grip.OwnerID;        //
            m_drag.type   = grip.GripType;       //  

            auto& line = static_cast<LineEntity*>(obj)->GetLine();
             
            m_drag.base = { line.Start,line.End };
        } 

        m_dragging  = true;
        m_activeIdx = idx;
        return true;                 // 消费事件
    }

    bool GripEditor::OnMouseMove(const InputEvent& e)
    {
        XMFLOAT2 sp((float)e.MouseX, (float)e.MouseY);
        const Camera* cam = m_scene->GetCamera();

        // 更新 hover（无论是否拖拽）
        m_hoveredIdx = m_gripManager->HitTest(sp, cam);

        if (!m_dragging) return false; 

        auto worldPos = cam->ScreenToWorld(sp.x,sp.y);    
         
        auto obj = m_scene->GetEntity(m_drag.id);

        if (obj->IsKindOf<LineEntity>())
        { 
            auto line = static_cast<LineEntity*>(obj);

            // 如果是捕获的消息则使用捕获的坐标。
            if (e.HasSnap)
                worldPos = e.SnapWorld;
            
            auto newLine = MoveGrip(m_drag.base, m_drag.type, worldPos);

            line->SetLine({ newLine.Start,newLine.End });
        } 

        m_drag.active = false; 

        return true;
    }

    
    bool GripEditor::OnMouseUp(const InputEvent& e)
    {
        if (e.Button != MouseButton::Left || !m_dragging) return false;

        auto obj = m_scene->GetEntity(m_drag.id);

        if (obj->IsKindOf<LineEntity>())
        {
            auto& line = static_cast<LineEntity*>(obj)->GetLine();

            LineSegment    after = { line.Start,line.End };

            // 执行压栈操作 
            m_cmdStack->Push(std::make_unique<DragLineCommand>(m_drag.id, m_drag.base, after));
        } 

        m_dragging  = false;
        m_activeIdx = -1;  

        return true;
    }
     
    DirectX::XMFLOAT3 GripEditor::GetDragStart() const
    {
        if (!m_dragging)
            return {};

        // LineSegment base → 转成 point
        return m_drag.base.Start; // 或 end，看你拖的是哪个点
    }
}