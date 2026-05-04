#pragma once
#include "App/Scene/Scene.h"
#include "App/Input/InputEvent.h"
#include "App/CommandStack/CommandStack.h"
#include "Render/Viewport/Viewport.h"
#include "App/Picking/Picking.h"

namespace MiniCAD
{
    struct LineSegment
    {
        DirectX::XMFLOAT3 Start;
        DirectX::XMFLOAT3 End;
    };

    struct Grip
    { 
        enum class Type : uint8_t
        {
            Start,
            Mid,
            End,
            Corner,     // 多段线
            Center,     // CAD 圆心
            Tangent     // 曲线控制点
        };

        Object::ObjectID   OwnerID;
        Type               GripType;
        DirectX::XMFLOAT3  WorldPos;
    };

    struct DragState
    {
        struct Entry
        {
            Object::ObjectID Id;
            Grip::Type       Type;

            enum class Kind
            {
                Line,
                Point
            } Kind;

            LineSegment BaseLine;
            XMFLOAT3    BasePoint;
        };

        std::vector<Entry> Entries;
        bool Active = false;
        XMFLOAT3 DirtyBase = { 0,0,0 };

        void Clear()
        {
            Entries.clear();
            Active = false;
            DirtyBase = { 0,0,0 };
        }
    };

    class GripEditor
    {
    public:
        GripEditor(Viewport& viewport, Scene& scene, CommandStack& cmdStack, Picking& picking)
            : m_viewport(viewport)
            , m_scene(scene)
            , m_cmdStack(cmdStack)
            , m_picking(picking)
        {
        }

    public:
        bool OnInput     (const InputEvent& e);
        bool IsDragging  () const { return m_dragging; }
        bool IsGripsEmpty() const { return m_grips.empty(); }
        void ReBuildGrip ()       { Rebuild(); };

        const std::vector<Grip>& GetGrips()    const { return m_grips; }       // 获取夹点 
        const std::vector<int>& HoveredGrips() const { return m_hoveredIdxs; }

        void MarkDirty() { m_dirty = true; }   // selection 变化后由 Editor 通知
        DirectX::XMFLOAT3 GetDragBase() const;

    public:
        // 拖动时返回所有对象的原始几何快照，用于渲染灰色ghost
        const std::vector<DragState::Entry>& GetDragEntries() const { return m_drag.Entries; }
        void CancelDrag();
    private:
        bool OnMouseDown(const InputEvent& e);
        bool OnMouseMove(const InputEvent& e);
        bool OnMouseUp  (const InputEvent& e);

    private:
        // 仅在 selection 变化时重建（内部比较上次 selection）
        bool Rebuild();
        int  HitTest(const DirectX::XMFLOAT2& screenPt, float thresh = 8.f) const; 
        std::vector<int> HitTestAll(const DirectX::XMFLOAT2& screenPt, float thresh = 8.f) const;
        void UpdateGripPos(Object::ObjectID id, const LineSegment& seg);          // 辅助更新夹点

    private:
        LineSegment MoveGrip(const LineSegment& seg, Grip::Type type, const DirectX::XMFLOAT3& p);

    private:
        Scene&             m_scene;
        Viewport&          m_viewport;
        CommandStack&      m_cmdStack;
        Picking&           m_picking;                           
        DragState          m_drag       = { };
        bool               m_dragging   = false;
        int                m_activeIdx  = -1;
        std::vector<int>   m_hoveredIdxs;
        std::vector<Grip>  m_grips;
        bool               m_dirty    = true;   // true = 需要重建夹点  
    };
}
