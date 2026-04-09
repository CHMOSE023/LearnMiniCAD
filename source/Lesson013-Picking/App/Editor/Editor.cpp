#include "Editor.h"
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h"  
#include "App/Input/InputEvent.h" 
#include <App/Tools/LineTool.h>

namespace MiniCAD
{
    Editor::Editor(Scene* scene, CommandStack* cmdStack)
        : m_scene(scene)
        , m_cmdStack(cmdStack)
		, m_picking(scene)
    {
    }

    /// <summary>
    /// 责任链模式，消息处理有优先级
    /// </summary> 
    bool Editor::OnInput(const InputEvent& e)
    { 
        // 1️ 全局按键优先处理
        if (e.Type == InputEventType::KeyDown && e.KeyCode == VK_ESCAPE)
        { 
            if (m_tool)
            {
                m_tool->Cancel();
                m_tool.reset();
                printf("[Editor] ESC exit tool\n");
                return true;
            }
        }

        // 2️ 工具处理
        if (m_tool)
        {
            switch (e.Type)
            {
            case InputEventType::MouseButtonDown: m_tool->OnMouseDown(e); return true;
            case InputEventType::MouseButtonUp:   m_tool->OnMouseUp(e);   return true;
            case InputEventType::MouseMove:       m_tool->OnMouseMove(e); OnMouseMove(e); return true; 
            case InputEventType::KeyDown:         m_tool->OnKeyDown(e);   return true;
            case InputEventType::MouseWheel:      OnMouseWheel(e);        return true;
            default: return false;
            } 
        }

        // 3️ 默认 Editor 自己处理
        switch (e.Type)
        {
        case InputEventType::MouseButtonDown: OnMouseButtonDown(e); return true;
        case InputEventType::MouseButtonUp:   OnMouseButtonUp(e);   return true;
        case InputEventType::MouseWheel:      OnMouseWheel(e);      return true;
        case InputEventType::MouseMove:       OnMouseMove(e);       return true;
        case InputEventType::KeyDown:         OnKeyDown(e);         return true;
        default:
            return false;
        } 
        return false;
    }

    void Editor::OnResize(float width, float height)
    {
        m_scene->GetCamera()->Resize(width, height);
    }

    const std::unordered_set<Object::ObjectID>& Editor::GetSelection()
    {
		return m_picking.GetSelection();
    }
    
    const std::unordered_set<Object::ObjectID>& Editor::GetHovered()
    {
        return m_picking.GetHovered();
    }


    void Editor::OnKeyDown(const InputEvent& e)
    {  
        if (e.KeyCode == 'L')
        { 
            m_tool = std::make_unique<LineTool>(m_scene, m_cmdStack);
            return;
        }

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
		
        // 释放命令
        if (m_tool&& e.KeyCode == VK_ESCAPE)
        {
            m_tool->Cancel();
            m_tool.reset();
            printf("[Editor] ESC exit tool\n");
            return ;
        }
    }

    void Editor::OnKeyUp(const InputEvent& e)
    {
    }

    void Editor::OnMouseButtonUp(const InputEvent& e)
    {
        m_picking.OnMouseUp(e);
    }

    void Editor::OnMouseButtonDown(const InputEvent& e)
    {
        m_picking.OnMouseDown(e); 

       // auto world =  m_scene->GetCamera()->ScreenToWorld(e.MouseX,e.MouseY);
       // XMFLOAT2 screen = m_scene->GetCamera()->WorldToScreen(world);
	   // printf("[Editor] MouseDown at screen (%0.2f, %0.2f), world (%.2f, %.2f)\n", screen.x, screen.y, world.x, world.y);

    }
    void Editor::OnMouseMove(const InputEvent& e)
    { 
        if (e.IsMouseButtonDown(MouseButton::Middle))
        { 
            m_scene->GetCamera()->Pan(e.MouseX - e.LastMouseX, e.MouseY - e.LastMouseY);
        }
		
        m_picking.OnMouseMove(e);
    }

	// 鼠标滚轮：缩放
    void Editor::OnMouseWheel(const InputEvent& e)
    {
		m_scene->GetCamera()->Zoom(e.WheelDelta, e.MouseX, e.MouseY);
    }
     
   
}
