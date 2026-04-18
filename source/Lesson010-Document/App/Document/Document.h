#pragma once
#include "App/Scene/Scene.h" 
#include "App/Editor/Editor.h"
#include "App/CommandStack/CommandStack.h" 
#include "App/Input/IInputHandler.h" 
#include "Render/Viewport/Viewport.h"  
#include "Render/D3D11/Renderer.h" 
#include "Core/Object/Object.hpp"
#include <unordered_set>
#include <string>
#include <memory>
namespace MiniCAD
{
    class Document  : public IInputHandler
    {
    public:
        Document(Renderer& render,float width = 600, float height = 400);

        // ── IEventHandler ─────────────────────────────────────
        bool OnInput(const InputEvent& e)  override;
         
        // ── 视图与呈现 ──────────────────────────────────────────
        void Resize(float width, float height);
        void Render(const RenderTarget& target);

        // ── 数据访问 ──────────────────────────────────────────
        Scene&              GetScene()              { return m_scene; }
        const Scene&        GetScene()  const       { return m_scene; }

        Editor&             GetEditor()             { return m_editor; }
        const  Editor&      GetEditor() const       { return m_editor; }

        Viewport&           GetViewport()           { return m_viewport; }
        const  Viewport&    GetViewport() const     { return m_viewport; }

        CommandStack&       GetCommandStack()       { return m_cmdStack; }
        const CommandStack& GetCommandStack() const { return m_cmdStack; }

        LayerManager&       GetLayerManager()       { return m_scene.GetLayerManager(); }
        const LayerManager& GetLayerManager() const { return m_scene.GetLayerManager(); }

        // ── Undo / Redo ───────────────────────────────────────
        void Undo()          { m_cmdStack.Undo(m_scene); }
        void Redo()          { m_cmdStack.Redo(m_scene); }
        bool CanUndo() const { return m_cmdStack.CanUndo();}
        bool CanRedo() const { return m_cmdStack.CanRedo();}
  
    private:
        Scene          m_scene;
        CommandStack   m_cmdStack;
        Editor         m_editor;
        Viewport       m_viewport;
    };
}
 
 