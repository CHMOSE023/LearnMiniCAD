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
        switch (e.type)
        {
        case InputEventType::MouseButtonDown:
            OnMouseButtonDown(e);
            return true;
        case InputEventType::MouseMove:
            OnMouseMove(e);      // 新增
            return false;        // 不消费，Viewport 也需要
        case InputEventType::KeyDown:
            OnKeyDown(e);
            return true;
        default:
            return false;
        } 
        return false;
    }


    void Editor::OnKeyDown(const InputEvent& e)
    {  
        // Ctrl+Z：Undo
        if (e.HasModifier(ModifierKey::Ctrl) && e.keyCode == 'Z')
        {
            m_cmdStack->Undo(*m_scene); 
            return;
        }

        // Ctrl+Y：Redo
        if (e.HasModifier(ModifierKey::Ctrl) && e.keyCode == 'Y')
        {
            m_cmdStack->Redo(*m_scene); 
            return;
        }
    }
 

    void Editor::OnMouseMove(const InputEvent& e)
    {
        
    }


    void Editor::OnMouseButtonDown(const InputEvent& e)
    {
       

    }
     
}
