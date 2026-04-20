#include "Picking.h"
namespace MiniCAD
{
    // ───────────────── 工具函数 ─────────────────
    static float PointToSegmentDist(const XMFLOAT2& p, const XMFLOAT2& a, const XMFLOAT2& b)
    {
        float dx    = b.x - a.x, dy = b.y - a.y;
        float lenSq = dx * dx + dy * dy;

        if (lenSq < 1e-6f)
            return std::hypot(p.x - a.x, p.y - a.y);

        float t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / lenSq;
        t = std::max(0.f, std::min(1.f, t));

        float cx = a.x + t * dx;
        float cy = a.y + t * dy;
        return std::hypot(p.x - cx, p.y - cy);
    }

    // 线段相交（用于框选修复）
    static bool SegmentIntersect(const XMFLOAT2& p1, const XMFLOAT2& p2,  const XMFLOAT2& q1, const XMFLOAT2& q2)
    {
        auto cross = [](const XMFLOAT2& a, const XMFLOAT2& b, const XMFLOAT2& c)
            {
                return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            };

        float d1 = cross(p1, p2, q1);
        float d2 = cross(p1, p2, q2);
        float d3 = cross(q1, q2, p1);
        float d4 = cross(q1, q2, p2);

        return (d1 * d2 < 0) && (d3 * d4 < 0);
    }

    static bool SegmentIntersectsRect(const XMFLOAT2& a, const XMFLOAT2& b,  float xMin, float yMin, float xMax, float yMax)
    {
        auto inRect = [&](const XMFLOAT2& p)
            {
                return p.x >= xMin && p.x <= xMax && p.y >= yMin && p.y <= yMax;
            };

        if (inRect(a) || inRect(b)) return true;

        XMFLOAT2 r1{ xMin, yMin }, r2{ xMax, yMin };
        XMFLOAT2 r3{ xMax, yMax }, r4{ xMin, yMax };

        return SegmentIntersect(a, b, r1, r2) ||
               SegmentIntersect(a, b, r2, r3) ||
               SegmentIntersect(a, b, r3, r4) ||
               SegmentIntersect(a, b, r4, r1);
    }

    // ───────────────── 构造 ─────────────────
    Picking::Picking(Scene& scene, Viewport& viewport)
        : m_scene(scene)
        , m_viewport(viewport)
    {
    }

    // ───────────────── 输入入口 ─────────────────

    bool Picking::OnInput(const InputEvent& e)
    { 
        switch (e.Type)
        {
        case InputEventType::MouseButtonDown:
            if (e.Button == MouseButton::Left) { OnMouseDown(e); return true; }
            break;

        case InputEventType::MouseMove:
            OnMouseMove(e); return true;

        case InputEventType::MouseButtonUp:
            if (e.Button == MouseButton::Left) { OnMouseUp(e); return true; }
            break;

        case InputEventType::KeyDown:
            OnKeyDown(e); return true;

        default:
            break;
        }
        return false;
    }


    // ───────────────── 查询接口 ─────────────────

    Picking::ObjectID Picking::HitTest(const XMFLOAT2& pt, float thresh)
    {
        ObjectID best = Object::InvalidID;
        float bestDist = FLT_MAX;
        auto camera = m_viewport.GetCamera();

        m_scene.ForEachObject([&](const Object& obj)
            {
                if (auto line = dynamic_cast<const LineEntity*>(&obj))
                {
                    auto a = camera.WorldToScreen(line->GetLine().Start);
                    auto b = camera.WorldToScreen(line->GetLine().End);

                    float d = PointToSegmentDist(pt, a, b);
                    if (d < thresh && d < bestDist)
                    {
                        bestDist = d;
                        best = obj.GetID();
                    }
                }
            });

        return best;
    }

    std::unordered_set<Picking::ObjectID>  Picking::BoxSelect(const XMFLOAT2& a, const XMFLOAT2& b)
    {
        float xMin = std::min(a.x, b.x);
        float xMax = std::max(a.x, b.x);
        float yMin = std::min(a.y, b.y);
        float yMax = std::max(a.y, b.y);

        bool fullyContain = (b.x > a.x);
        auto camera = m_viewport.GetCamera();

        std::unordered_set<ObjectID> result;

        m_scene.ForEachObject([&](const Object& obj)
        {
            if (auto line = dynamic_cast<const LineEntity*>(&obj))
            {
                auto s = camera.WorldToScreen(line->GetLine().Start);
                auto e = camera.WorldToScreen(line->GetLine().End);

                bool hit = fullyContain
                    ? (s.x >= xMin && s.x <= xMax && s.y >= yMin && s.y <= yMax &&
                        e.x >= xMin && e.x <= xMax && e.y >= yMin && e.y <= yMax)
                    : SegmentIntersectsRect(s, e, xMin, yMin, xMax, yMax);

                if (hit)
                    result.insert(obj.GetID());
            }
        });

        return result;
    }

    // ───────────────── 输入处理 ─────────────────

    void Picking::OnMouseDown(const InputEvent& e)
    {
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
        if (m_drag == DragState::Pressing)
            DoPointPick(e);
        else if (m_drag == DragState::BoxSelecting)
            DoBoxPick(e);

        m_drag = DragState::Idle;
    }

    void Picking::OnKeyDown(const InputEvent& e)
    {
        if (e.IsCancel())
            m_selection.clear();
    }

    // ───────────────── 内部逻辑 ─────────────────

    void Picking::UpdateHovered(const InputEvent& e)
    {
        XMFLOAT2 pt{ (float)e.MouseX, (float)e.MouseY };

        ObjectID id = HitTest(pt, HOVER_THRESH);
       
        if (id<=0)
        {
            return;
        }

        printf("Picking::UpdateHovered ObjectId %d \n",id);

        if (m_hovered.size() == 1 && *m_hovered.begin() == id)
            return;

        m_hovered.clear();
        if (id != Object::InvalidID)
            m_hovered.insert(id);

    }

    void Picking::DoPointPick(const InputEvent& e)
    {
        bool ctrl = e.HasModifier(ModifierKey::Ctrl);

        XMFLOAT2 pt{ (float)e.MouseX, (float)e.MouseY };
        ObjectID id = HitTest(pt, PICK_THRESH);

        if (id == Object::InvalidID)
        {
            if (!ctrl) m_selection.clear();
            return;
        }

        if (ctrl)
        {
            if (m_selection.count(id))
                m_selection.erase(id);
            else
                m_selection.insert(id);
        }
        else
        {
            m_selection = { id };
        }
    }

    void Picking::DoBoxPick(const InputEvent& e)
    {
        bool ctrl = e.HasModifier(ModifierKey::Ctrl);

        XMFLOAT2 a{ (float)m_pressX, (float)m_pressY };
        XMFLOAT2 b{ (float)e.MouseX, (float)e.MouseY };

        auto result = BoxSelect(a, b);

        if (ctrl)
        {
            m_selection.insert(result.begin(), result.end());
        }
        else
        {
            m_selection = std::move(result);
        }
    }

}