#include "Document.h"    
#include "Render/D3D11/Renderer.h"
#include "Core/Entity/LineEntity.hpp"
#include "Core/Object/Object.hpp"
#include <vector> 
#include <memory>
#include <utility>
namespace MiniCAD
{
    Document::Document(Renderer& render, float width, float height)
        : m_scene()
        , m_cmdStack()
        , m_overlay()
        , m_viewport(render, width, height)
        , m_picking(m_scene, m_viewport)
        , m_editor(m_scene, m_cmdStack, m_viewport, m_overlay,m_picking)
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
        m_overlayVertices.clear();

        UpdateSceneVerties();
           
        m_overlay.ToVertices(m_overlayVertices);      // 每帧分配

        auto hoverIds     = m_picking.GetHovered();   // 获取悬浮 ids
        auto selectionIds = m_picking.GetSelection(); // 获取选中 ids

        //  悬浮或选中的在overlayVertices设置颜色  
        auto vs = BuildViewState();
        m_viewport.Render(target, vs);
         
    } 
     

    void Document::UpdateSceneVerties()
    { 
        if (!m_scene.IsDirty() && !m_picking.IsDirty())
            return;


        m_sceneVertices.clear();
        m_overlay.Clear();  

        const auto& hoverIds     = m_picking.GetHovered();
        const auto& selectionIds = m_picking.GetSelection();

        const DirectX::XMFLOAT4 hoverColor     = { 0,  0.5, 0.8, 0.9 };
        const DirectX::XMFLOAT4 selectionColor = { 0,  0.3, 0.8, 0.9 };

        m_scene.ForEachObject([&](const Object& obj)
            {
                if (!obj.IsKindOf<LineEntity>())
                    return;

                const auto& line = static_cast<const LineEntity&>(obj);
                const auto& attr = line.GetAttr();
                const auto& geom = line.GetLine();

                const auto id = obj.GetID();

                const bool isSelected = selectionIds.contains(id);
                const bool isHovered = hoverIds.contains(id);

                // ===== Base：只画普通 =====
                if (!isSelected && !isHovered)
                {
                    m_sceneVertices.push_back({ geom.Start, attr.Color });
                    m_sceneVertices.push_back({ geom.End,   attr.Color });
                }

                // ===== Overlay：画高亮 =====
                if (isSelected)
                { 
                    m_overlay.AddLine(geom.Start, geom.End, selectionColor);
                }
                else if (isHovered)
                { 
                    m_overlay.AddLine(geom.Start, geom.End, hoverColor);
                }
            });

        m_scene.ClearDirty();
        m_picking.ClearDirty(); 
    }

    ViewState Document::BuildViewState()
    {
        ViewState vs;

        vs.Scene   = m_sceneVertices;
        vs.Overlay = m_overlayVertices;

        vs.Selection.Active = m_picking.IsBoxSelecting();
        vs.Selection.Start  = m_picking.GetBoxStart();
        vs.Selection.End    = m_picking.GetBoxEnd();  

        vs.ShowGrid  = true;
        vs.ShowGizmo = true;

        auto selectids =  m_picking.GetSelection();
        // 选中的夹点
        for (const auto id : selectids)
        {
            auto entity = m_scene.GetEntity(id);
            if (!entity) continue; 

            if (entity->IsKindOf<LineEntity>())
            {
                auto line = static_cast<LineEntity*>(entity); 
                auto camera = m_viewport.GetCamera();  

                GripDraw g1, g2, g3;
                g1.Pos     = camera.WorldToScreen(line->GetLine().Start);;
                g1.Type    = GripDraw::Type::Start;
                g1.Hovered = false;
                 
                g2.Pos     = camera.WorldToScreen(line->GetLine().End);
                g2.Type    = GripDraw::Type::End;
                g2.Hovered = false; 

                g3.Pos     = camera.WorldToScreen(line->GetLine().Midpoint());
                g3.Type    = GripDraw::Type::Mid;
                g3.Hovered = false;

                vs.Grips.push_back(g1);
                vs.Grips.push_back(g2);
                vs.Grips.push_back(g3);
            }

        } 

        return vs;
    }
 
}