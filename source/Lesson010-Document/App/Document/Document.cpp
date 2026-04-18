#include "Document.h"   
namespace MiniCAD
{
	Document::Document(Renderer& render, float width, float height)
        : m_scene()
        , m_cmdStack()
        , m_editor(m_scene, m_cmdStack)
        , m_viewport(render, width, height)
	{  
	}

    bool Document::OnInput(const InputEvent& e)
    { 
        switch (e.Type)
        {
        case InputEventType::MouseMove:
            // 平移相机
            if (e.IsMouseButtonDown(MouseButton::Middle))
            {
                m_viewport.Pan(e.MouseX - e.LastMouseX, e.MouseY - e.LastMouseY);
            } 
            m_editor.OnInput(e);
            return false;  
        case InputEventType::MouseWheel:
        { 
            m_viewport.Zoom(e.WheelDelta, e.MouseX, e.MouseY);
            m_editor.OnInput(e);
            return false;
        }
        default:
            return   m_editor.OnInput(e);
        } 

        return false;
    }

    void Document::Resize(float width, float height)
    {
        m_viewport.Resize(width, height);  
    }

    void Document::Render( const RenderTarget & target)
    {
        m_viewport.Render(target);
    }
}