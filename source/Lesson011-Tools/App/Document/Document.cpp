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
        UpdateSceneVerties();
        m_viewport.Render(target, m_sceneVertices);
    }

    void Document::UpdateSceneVerties()
    {
        if (m_scene.IsDirty()) // 如果场景数据发送变换 则更新数据
        {
            m_sceneVertices.clear();

            m_scene.ForEachObject([&](const Object& obj)
                {
                    if (obj.IsKindOf<LineEntity>())
                    { 
                        const auto& line  = static_cast<const LineEntity&>(obj); 
                        const auto& color = line.GetAttr().Color;
                        const auto& start = line.GetLine().Start;
                        const auto& end   = line.GetLine().End;

                        m_sceneVertices.push_back({ start, color });
                        m_sceneVertices.push_back({ end,   color });
                    } 

                });

            m_scene.ClearDirty();// 清空创建

        }


    }
}