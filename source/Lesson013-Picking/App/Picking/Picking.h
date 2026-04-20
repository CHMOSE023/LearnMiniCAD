#pragma once
#include <DirectXMath.h>
#include <unordered_set>
#include <vector>
#include "App/Input/InputEvent.h"
#include "App/Scene/Scene.h"
#include "Core/Object/Object.hpp" 
#include "Render/Viewport/Viewport.h"  
namespace MiniCAD
{
    class Picking
    {
    public:
        using ObjectID = Object::ObjectID;  
        Picking(Scene& scene, Viewport& viewport);
         
        bool OnInput(const InputEvent& e);    // 输入入口 

        // 查询接口（给 Tool / Editor 用）
        ObjectID                     HitTest  (const DirectX::XMFLOAT2& pt, float thresh);
        std::unordered_set<ObjectID> BoxSelect(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b);

        bool              IsBoxSelecting() const { return m_drag == DragState::BoxSelecting; } 
        DirectX::XMFLOAT3 GetDragStart()   const { return { (float)m_pressX, (float)m_pressY ,0.0 }; }

        // 状态访问
        const std::unordered_set<ObjectID>& GetSelection() const { return m_selection; }
        const std::unordered_set<ObjectID>& GetHovered()  const { return m_hovered; }


    private:
        void OnMouseDown  (const InputEvent& e);
        void OnMouseMove  (const InputEvent& e);
        void OnMouseUp    (const InputEvent& e);
        void OnKeyDown    (const InputEvent& e);

        void UpdateHovered(const InputEvent& e);
        void DoPointPick  (const InputEvent& e);
        void DoBoxPick    (const InputEvent& e);

    private:
        enum class DragState : uint8_t
        {
            Idle,
            Pressing,
            BoxSelecting
        };

        static constexpr float DRAG_THRESH  = 2.0f;
        static constexpr float HOVER_THRESH = 6.0f;
        static constexpr float PICK_THRESH  = 5.0f;

        Scene&    m_scene;
        Viewport& m_viewport;

        std::unordered_set<ObjectID> m_selection;
        std::unordered_set<ObjectID> m_hovered;

        DragState m_drag = DragState::Idle;
        int m_pressX = 0;
        int m_pressY = 0;
    };
}