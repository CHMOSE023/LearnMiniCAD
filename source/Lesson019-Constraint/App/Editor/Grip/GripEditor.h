#pragma once
#include "GripManager.h" 
#include "App/Input/InputEvent.h"
#include "App/CommandStack/CommandStack.h"
#include "App/Editor/Grip/MoveGrip.h"

namespace MiniCAD
{
    // 拖动状态
    struct DragState
    {
        Object::ObjectID id;
        Grip::Type type;

        LineSegment base;   // 固定基准（最关键）
        LineSegment current;

        bool active = false;
    };

    class GripEditor
    {
    public:
        GripEditor(Scene* scene, GripManager* gripManager, CommandStack* cmdStack)
            : m_scene(scene)
            , m_gripManager(gripManager)
            , m_cmdStack(cmdStack)
        {
        }

        // 返回 true 表示事件已被夹点编辑器消费，Picking 不应再处理
        bool OnMouseDown(const InputEvent& e);
        bool OnMouseMove(const InputEvent& e);
        bool OnMouseUp  (const InputEvent& e);

        bool IsDragging() const { return m_dragging; }

        // 供渲染器查询：当前悬停的夹点索引（-1 表示无）
        int  HoveredGrip() const { return m_hoveredIdx; }

        DirectX::XMFLOAT3 GetDragStart() const;
    private:
        void ApplyDrag(const DirectX::XMFLOAT3& worldPos); 

        Scene*        m_scene = nullptr;
        GripManager*  m_gripManager = nullptr;
        CommandStack* m_cmdStack;

        DragState     m_drag;
        bool      m_dragging   = false;
        int       m_activeIdx  = -1;      // 正在拖拽的夹点索引
        int       m_hoveredIdx = -1;      // 鼠标悬停的夹点索引

    };
}