#include "GripEditor.h"
#include "Core/Entity/LineEntity.hpp"
#include "App/Command/DragLineCommand.h"
#include <memory>
using namespace DirectX;

namespace MiniCAD
{
    // ─────────────────────────────────────────────
    //  OnInput  入口
    // ─────────────────────────────────────────────
    bool GripEditor::OnInput(const InputEvent& e)
    {
        // 拖拽进行中不重建，避免 m_grips 被实时修改的线段数据污染
        if (!m_dragging)
        {
            if (!Rebuild())
                return false;
        }

        if (m_grips.empty())
            return false;

        switch (e.Type)
        {
        case InputEventType::MouseButtonDown:
            if (e.Button == MouseButton::Left)
                return OnMouseDown(e);  // 命中夹点才消费，否则让 Picking 处理
            break;

        case InputEventType::MouseMove:
            return OnMouseMove(e);      // 由函数自身决定是否消费

        case InputEventType::MouseButtonUp:
            if (e.Button == MouseButton::Left)
                return OnMouseUp(e);
            break;

        default:
            break;
        }

        return false;
    }

    // ─────────────────────────────────────────────
    //  MouseDown  — 命中夹点则开始拖拽
    // ─────────────────────────────────────────────
    bool GripEditor::OnMouseDown(const InputEvent& e)
    {
        XMFLOAT2 sp((float)e.MouseX, (float)e.MouseY);
        auto hits = HitTestAll(sp);
        if (hits.empty()) return false;

        m_drag.Clear();
        m_drag.Active    = true;

        auto hit = HitTest(sp);
        m_drag.DirtyBase = m_grips[hit].WorldPos; 

        for (int idx : hits)
        {
            const Grip& grip = m_grips[idx];
            auto obj = m_scene.GetEntity(grip.OwnerID);
            if (!obj || !obj->IsKindOf<LineEntity>()) continue;

            auto& L = static_cast<LineEntity*>(obj)->GetLine();
            m_drag.Entries.push_back({ grip.OwnerID, grip.GripType, { L.Start, L.End } });
        }

        if (m_drag.Entries.empty()) return false;

        m_dragging = true;
        m_activeIdx = hits[0];
        return true;
    }

    // ─────────────────────────────────────────────
    //  MouseMove  — 更新 hover；拖拽中实时移动线段
    // ─────────────────────────────────────────────
    bool GripEditor::OnMouseMove(const InputEvent& e)
    {
        XMFLOAT2 sp((float)e.MouseX, (float)e.MouseY);
        m_hoveredIdxs = HitTestAll(sp);
        if (!m_dragging) return false;

        XMFLOAT3 worldPos = e.HasSnap
            ? e.SnapWorld
            : m_viewport.GetCamera().ScreenToWorld(sp.x, sp.y);

        for (auto& entry : m_drag.Entries)
        {
            auto obj = m_scene.GetEntity(entry.Id);
            if (!obj || !obj->IsKindOf<LineEntity>()) continue;

            // 各自基于自己的 Base + 自己的 Type 计算，互不影响
            auto newSeg = MoveGrip(entry.Base, entry.Type, worldPos);
            static_cast<LineEntity*>(obj)->SetLine({ newSeg.Start, newSeg.End });

            for (auto& grip : m_grips)
            {
                if (grip.OwnerID != entry.Id) continue;
                switch (grip.GripType)
                {
                case Grip::Type::Start: grip.WorldPos = newSeg.Start; break;
                case Grip::Type::End:   grip.WorldPos = newSeg.End;   break;
                case Grip::Type::Mid:
                    grip.WorldPos = { (newSeg.Start.x + newSeg.End.x) * 0.5f,
                                      (newSeg.Start.y + newSeg.End.y) * 0.5f, 0.f };
                    break;
                }
            }
        }

        m_scene.MarkDirty();
        return true;
    }

    void GripEditor::UpdateGripPos(Object::ObjectID id, const LineSegment& seg)
    {
        for (auto& grip : m_grips)
        {
            if (grip.OwnerID != id) continue;
            switch (grip.GripType)
            {
            case Grip::Type::Start: grip.WorldPos = seg.Start; break;
            case Grip::Type::End:   grip.WorldPos = seg.End;   break;
            case Grip::Type::Mid:
                grip.WorldPos = { (seg.Start.x + seg.End.x) * 0.5f,
                                  (seg.Start.y + seg.End.y) * 0.5f, 0 };
                break;
            }
        }
    }

    // ─────────────────────────────────────────────
    //  MouseUp  — 提交命令到 CommandStack
    // ─────────────────────────────────────────────
    bool GripEditor::OnMouseUp(const InputEvent& e)
    {
        if (!m_dragging) return false;

        if (m_drag.Entries.size() == 1)
        {
            // 单夹点，用原来的命令
            auto& entry = m_drag.Entries[0];
            auto obj = m_scene.GetEntity(entry.Id);
            if (obj && obj->IsKindOf<LineEntity>())
            {
                auto& L = static_cast<LineEntity*>(obj)->GetLine();
                m_cmdStack.Push(std::make_unique<DragLineCommand>(entry.Id, entry.Base, LineSegment{ L.Start, L.End }));
            }
        }
        else
        {
            // 多夹点重叠，用组合命令，一次 Undo 全部回退
            std::vector<DragMultiLineCommand::Entry> entries;
            for (auto& entry : m_drag.Entries)
            {
                auto obj = m_scene.GetEntity(entry.Id);
                if (!obj || !obj->IsKindOf<LineEntity>()) continue;
                auto& L = static_cast<LineEntity*>(obj)->GetLine();
                entries.push_back({ entry.Id, entry.Base, { L.Start, L.End } });
            }
            if (!entries.empty())
                m_cmdStack.Push(std::make_unique<DragMultiLineCommand>(std::move(entries)));
        }

        m_dragging = false;
        m_activeIdx = -1;
        m_drag.Clear();
        return true;
    }

    // ─────────────────────────────────────────────
    //  MoveGrip  — 基于 Base 快照计算新线段
    // ─────────────────────────────────────────────
    LineSegment GripEditor::MoveGrip(const LineSegment& seg,  Grip::Type type,  const XMFLOAT3& p)
    {
        LineSegment out = seg;  // 从 Base 复制，避免误差累积

        switch (type)
        {
        case Grip::Type::Start:
            out.Start = p;
            break;

        case Grip::Type::End:
            out.End = p;
            break;

        case Grip::Type::Mid:
        {
            XMFLOAT3 mid{
                (seg.Start.x + seg.End.x) * 0.5f,
                (seg.Start.y + seg.End.y) * 0.5f,
                0.0f
            };
            float dx = p.x - mid.x;
            float dy = p.y - mid.y;
            out.Start.x += dx;  out.Start.y += dy;
            out.End.x += dx;  out.End.y += dy;
            break;
        }
        }

        return out;
    }

    // ─────────────────────────────────────────────
    //  Rebuild  — 仅在 selection 发生变化时重建夹点
    // ─────────────────────────────────────────────
    bool GripEditor::Rebuild()
    {
        if (!m_dirty)
            return !m_grips.empty();   // 未脏，直接用缓存

        m_dirty = false;
        m_grips.clear();

        auto selectionIds = m_picking.GetSelection();
        if (selectionIds.empty())
            return false;

        for (auto objId : selectionIds)
        {
            auto obj = m_scene.GetEntity(objId);
            if (!obj->IsKindOf<LineEntity>())
                continue;

            auto* line = static_cast<const LineEntity*>(obj);
            auto& L = line->GetLine();

            m_grips.push_back({ objId, Grip::Type::Start, L.Start });
            m_grips.push_back({ objId, Grip::Type::Mid,   L.Midpoint() });
            m_grips.push_back({ objId, Grip::Type::End,   L.End });
        }

        return !m_grips.empty();
    }

    // ─────────────────────────────────────────────
    //  HitTest  — 屏幕坐标命中测试
    // ─────────────────────────────────────────────
    int GripEditor::HitTest(const XMFLOAT2& screenPt, float thresh) const
    {
        int   bestIdx = -1;
        float bestDist = FLT_MAX;

        for (int i = 0; i < (int)m_grips.size(); ++i)
        {
            XMFLOAT2 sc = m_viewport.GetCamera().WorldToScreen(m_grips[i].WorldPos);
            float d = std::hypot(screenPt.x - sc.x, screenPt.y - sc.y);

            if (d < thresh && d < bestDist)
            {
                bestDist = d;
                bestIdx = i;
            }
        }

        return bestIdx;
    }

    std::vector<int> GripEditor::HitTestAll(const XMFLOAT2& screenPt, float thresh) const
    {
        std::vector<int> results;
        for (int i = 0; i < (int)m_grips.size(); ++i)
        {
            XMFLOAT2 sc = m_viewport.GetCamera().WorldToScreen(m_grips[i].WorldPos);
            float d = std::hypot(screenPt.x - sc.x, screenPt.y - sc.y);
            if (d < thresh)
                results.push_back(i);
        }
        return results;
    }


    DirectX::XMFLOAT3 GripEditor::GetDragBase() const
    {
        if (!m_dragging)
            return {};
         
        return m_drag.DirtyBase;  // 拖动的基点
    }

    // 取消拖动
    void GripEditor::CancelDrag()
    {
        if (!m_dragging) return;

        // 把所有对象还原到 Base 快照
        for (auto& entry : m_drag.Entries)
        {
            auto obj = m_scene.GetEntity(entry.Id);
            if (!obj || !obj->IsKindOf<LineEntity>()) continue;
            static_cast<LineEntity*>(obj)->SetLine({ entry.Base.Start, entry.Base.End });
        }

        m_dragging = false;
        m_activeIdx = -1;
        m_drag.Clear();
        m_scene.MarkDirty();
    }

}  
