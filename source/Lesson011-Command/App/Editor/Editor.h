#pragma once   
#include "App/Input/IInputHandler.h" 
#include "App/Input/InputEvent.h" 
#include "App/Scene/Scene.h" 
#include "App/CommandStack/CommandStack.h" 
#include "Render/Viewport/Viewport.h"
#include <unordered_set> 
#include <DirectXMath.h>
#include <memory>
#include <App/Tools/ITool.h>

namespace MiniCAD
{ 
    class Editor : public IInputHandler
    {
    public:
        Editor(Scene* scene, CommandStack* cmdStack); 
        bool OnInput(const InputEvent& e) override;  
        void OnResize(float width, float height);
    private:
        void OnMouseButtonDown(const InputEvent& e);
        void OnMouseButtonUp  (const InputEvent& e);
        void OnKeyDown        (const InputEvent& e);
        void OnKeyUp          (const InputEvent& e);
        void OnMouseMove      (const InputEvent& e);   
        void OnMouseWheel     (const InputEvent& e);
    private: 
        Scene*        m_scene    = nullptr;      
        CommandStack* m_cmdStack = nullptr; 
        Viewport*     m_view     = nullptr;           

        std::unique_ptr<ITool>   m_tool;
    };
}