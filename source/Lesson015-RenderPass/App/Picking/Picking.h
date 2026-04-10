#pragma once
#include <DirectXMath.h>
#include "App/Scene/Scene.h" 
#include "Core/Object/Object.hpp" 
#include <vector>
#include <unordered_set> 
#include "App/Input/InputEvent.h"
namespace MiniCAD
{ 
	class Picking
	{
    public:
        using ObjectID = Object::ObjectID;

        Picking(Scene* scene) : m_scene(scene) {}
          
        void OnMouseDown(const InputEvent& e);
        void OnMouseUp  (const InputEvent& e);
        void OnMouseMove(const InputEvent& e);
        void OnKeyDown  (const InputEvent& e);

		bool IsBoxSelected() const { return m_drag == DragState::BoxSelecting; }
        
		// 返回对象ID
        const std::unordered_set<Object::ObjectID>& GetSelection()    const { return m_selection; }
        const std::unordered_set<Object::ObjectID>& GetHovered()      const { return m_hovered; }
		const DirectX::XMFLOAT2                     GetBoxPress()     const { return DirectX::XMFLOAT2(m_pressX, m_pressY); }

    private: 

        void UpdateHovered(const InputEvent& e);
        void DoPointPick  (const InputEvent& e);

        void UpdateBoxSelection(const InputEvent& e);

    private:

        enum class DragState : uint8_t {
            Idle,                      // 自由移动，只做 hover 检测。
			Pressing,                  // 左键按下了，但还没有超过 DRAG_THRESH 阈值，仍然只做 hover 检测。
			BoxSelecting               // 移动超过阈值，确认用户在拖框
        };

        static constexpr float DRAG_THRESH = 10.0f;   // px，超过阈值 进入框选

		Scene* m_scene = nullptr;

        std::unordered_set<Object::ObjectID> m_selection;
        std::unordered_set<Object::ObjectID> m_hovered;

        DragState m_drag   = DragState::Idle;

        int    m_pressX = 0;
        int    m_pressY = 0; 
	};
}