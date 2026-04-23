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
        , m_snap()
        , m_currentSnap() 
        , m_editor(m_scene, m_cmdStack, m_viewport, m_overlay,m_picking, m_snap, m_currentSnap)
    {
      
    }

    bool Document::OnInput(const InputEvent& e)
    {   
        m_mouseX = e.MouseX; // 保存鼠标位置
        m_mouseY = e.MouseY; 
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

        // 场景点
        vs.Scene            = m_sceneVertices;
        vs.Overlay          = m_overlayVertices; 

        // 选择范围框
        vs.Selection.Active = m_picking.IsBoxSelecting();
        vs.Selection.Start  = m_picking.GetBoxStart();
        vs.Selection.End    = m_picking.GetBoxEnd();  
        
        // 光标位置
        vs.MouseX    = static_cast<float>(m_mouseX);
        vs.MouseY    = static_cast<float>(m_mouseY);  

        // 辅助网格
        vs.ShowGrid  = true;  

        // 最近点 
        vs.Snap.SnapType = static_cast<SnapDraw::Type>(m_currentSnap.SnapType);
        vs.Snap.Pos      = m_viewport.GetCamera().WorldToScreen(m_currentSnap.WorldPos);
        m_currentSnap    = {};// 获取后清零有残留

        // 光标中间方框
        vs.ShowCurrorBox = !m_editor.IsAcitveTool();

        // 夹点
        vs.ShowGizmo = true;
        if (vs.ShowGizmo)
        { 
            auto& hoveredIdxs = m_editor.GetGipEditor().HoveredGrips();
            auto& grips = m_editor.GetGipEditor().GetGrips();

            for (int i = 0; i < (int)grips.size(); ++i)
            {
                const auto& g = grips[i];
                auto        s = m_viewport.GetCamera().WorldToScreen(g.WorldPos);
                auto        type = static_cast<GripDraw::Type>(g.GripType);

                // 在列表里找，而不是判断单个 index
                bool hovered = std::find(hoveredIdxs.begin(), hoveredIdxs.end(), i) != hoveredIdxs.end();

                vs.Grips.push_back({ s, type, hovered });
            }
        } 

        return vs;
    }
 
}