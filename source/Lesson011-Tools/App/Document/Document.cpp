#include "Document.h"   
namespace MiniCAD
{
    Document::Document(Renderer& render, float width, float height)
        : m_scene()
        , m_cmdStack()
        , m_viewport(render, width, height)
        , m_editor(m_scene, m_cmdStack,m_viewport)
    {
    }

    bool Document::OnInput(const InputEvent& e)
    {
        return m_editor.OnInput(e); 
    }

    void Document::Resize(float width, float height)
    {
        m_viewport.Resize(width, height);
    }

    void Document::Render(const RenderTarget& target)
    {
        m_viewport.Render(target);
    }
}