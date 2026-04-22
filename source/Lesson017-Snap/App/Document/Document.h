#pragma once
#include "App/Scene/Scene.h" 
#include "App/Editor/Editor.h"
#include "App/CommandStack/CommandStack.h" 
#include "App/Input/IInputHandler.h" 
#include "App/Input/InputEvent.h"
#include "App/Overlay/Overlay.h"
#include "App/Picking/Picking.h"
#include "Render/Viewport/Viewport.h"  
#include "Render/ViewState.h"
#include "Render/D3D11/Renderer.h" 
#include "Render/D3D11/RenderTarget.h" 
#include <vector>

namespace MiniCAD
{
	class Document : public IInputHandler
	{
	public:
		Document(Renderer& render, float width = 600, float height = 400);

		// ── IEventHandler ─────────────────────────────────────
		bool OnInput(const InputEvent& e) override;


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
		void Undo() { m_cmdStack.Undo(m_scene); }
		void Redo() { m_cmdStack.Redo(m_scene); }
		bool CanUndo() const { return m_cmdStack.CanUndo(); }
		bool CanRedo() const { return m_cmdStack.CanRedo(); }
	private:
		void UpdateSceneVerties(); // 更新屏幕渲染数据   
		ViewState BuildViewState();// 构建渲染状态
	private:
		Editor         m_editor;
		Scene          m_scene; 
		CommandStack   m_cmdStack;
		Viewport       m_viewport;
		Overlay        m_overlay; 
		Picking        m_picking;

	private: // 捕获最近点
		SnapEngine     m_snap;
		SnapResult     m_currentSnap;

	private:           // 鼠标位置
		int            m_mouseX = 0; 
		int            m_mouseY = 0;
	private: 
		std::vector<Vertex_P3_C4> m_sceneVertices;   // 场景数据 
		std::vector<Vertex_P3_C4> m_overlayVertices; // 预览数据
		std::vector<Vertex_P3_C4> m_gripVertices;    // 夹点数据
	};
}