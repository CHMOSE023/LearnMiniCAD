#pragma once
#include "GripManager.h"
//#include "Snap/SnapEngine.h"
#include "App/Input/InputEvent.h"

namespace MiniCAD
{
    class GripEditor
    {
    public:
        GripEditor(Scene* scene, GripManager* grips)
            : m_scene(scene)
            , m_grips(grips)
        {
        }

        // 返回 true 表示事件已被夹点编辑器消费，Picking 不应再处理
        bool OnMouseDown(const InputEvent& e);
        bool OnMouseMove(const InputEvent& e);
        bool OnMouseUp  (const InputEvent& e);

        bool IsDragging() const { return m_dragging; }

        // 供渲染器查询：当前悬停的夹点索引（-1 表示无）
        int  HoveredGrip() const { return m_hoveredIdx; }


    private:
        void ApplyDrag(const DirectX::XMFLOAT3& worldPos);

        Scene*       m_scene = nullptr;
        GripManager* m_grips = nullptr;


        bool      m_dragging   = false;
        int       m_activeIdx  = -1;      // 正在拖拽的夹点索引
        int       m_hoveredIdx = -1;      // 鼠标悬停的夹点索引

    };
}