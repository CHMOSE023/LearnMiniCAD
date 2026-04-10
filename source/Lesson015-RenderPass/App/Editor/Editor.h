#pragma once   
#include "App/Input/IInputHandler.h" 
#include "App/Input/InputEvent.h" 
#include "App/Picking/Picking.h"
#include "App/Scene/Scene.h" 
#include "App/CommandStack/CommandStack.h" 
#include "Render/Viewport/Viewport.h"
#include <unordered_set> 
#include <DirectXMath.h>
#include <memory>
#include <App/Tools/ITool.h>
#include "Render/ViewState.h"
namespace MiniCAD
{ 
    class Editor : public IInputHandler
    {
    public:
        Editor(Scene* scene, CommandStack* cmdStack); 
        bool OnInput(const InputEvent& e) override;  
        void OnResize(float width, float height);
        ViewState BuildViewState() const;
        const std::unordered_set<Object::ObjectID>& GetSelection();
        const std::unordered_set<Object::ObjectID>& GetHovered(); 
    private:
        void OnMouseButtonDown(const InputEvent& e);
        void OnMouseButtonUp  (const InputEvent& e);
        void OnKeyDown        (const InputEvent& e);
        void OnKeyUp          (const InputEvent& e);
        void OnMouseMove      (const InputEvent& e);   
        void OnMouseWheel     (const InputEvent& e);
    private: 
        Scene*        m_scene     = nullptr;      
        CommandStack* m_cmdStack  = nullptr; 
        Viewport*     m_view      = nullptr; 
        bool          m_showGrid  = true; // 网格
        bool          m_showGizmo = true; // 夹点

        std::unique_ptr<ITool>   m_tool;

        Picking       m_picking;

		float 	 m_mouseX = 0.f, m_mouseY = 0.f; // 鼠标位置（屏幕坐标）
         
    };
}