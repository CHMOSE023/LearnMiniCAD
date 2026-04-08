#include "Editor.h"
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h"  
#include "App/Input/InputEvent.h" 

namespace MiniCAD
{
    Editor::Editor(Scene* scene, CommandStack* cmdStack)
        : m_scene(scene)
        , m_cmdStack(cmdStack)
    {
    }

    bool Editor::OnInput(const InputEvent& e)
    { 
        switch (e.Type)
        { 
        case InputEventType::MouseButtonDown:
            OnMouseButtonDown(e);
            return true;
        case InputEventType::MouseButtonUp:
            OnMouseButtonUp(e);
            return true;
        case InputEventType::MouseWheel:
            OnMouseWheel(e);
            return true;
        case InputEventType::MouseMove:
            OnMouseMove(e);       
            return true;         
        case InputEventType::KeyDown:
            OnKeyDown(e);
            return true;
        default:
            return false;
        } 
        return false;
    }

    void Editor::OnResize(float width, float height)
    {
        m_scene->GetCamera()->Resize(width, height);
    }


    void Editor::OnKeyDown(const InputEvent& e)
    {  
        // Ctrl+Z：Undo
        if (e.HasModifier(ModifierKey::Ctrl) && e.KeyCode == 'Z')
        {
            m_cmdStack->Undo(*m_scene); 
            return;
        } 
        // Ctrl+Y：Redo
        if (e.HasModifier(ModifierKey::Ctrl) && e.KeyCode == 'Y')
        {
            m_cmdStack->Redo(*m_scene); 
            return;
        } 
		
    }

    void Editor::OnKeyUp(const InputEvent& e)
    {
    }

    void Editor::OnMouseButtonUp(const InputEvent& e)
    {
    }

    void Editor::OnMouseButtonDown(const InputEvent& e)
    {
    }
    void Editor::OnMouseMove(const InputEvent& e)
    { 
        if (e.IsMouseButtonDown(MouseButton::Middle))
        { 
            m_scene->GetCamera()->Pan(e.MouseX - e.LastMouseX, e.MouseY - e.LastMouseY);
        }
		
    }

	// 鼠标滚轮：缩放
    void Editor::OnMouseWheel(const InputEvent& e)
    {
		m_scene->GetCamera()->Zoom(e.WheelDelta, e.MouseX, e.MouseY);
    }
     
   
}
