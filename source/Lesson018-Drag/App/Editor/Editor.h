#pragma once
#include "App/Input/IInputHandler.h" 
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h" 
#include "App/Tools/ITool.h"
#include "App/Overlay/Overlay.h"
#include "App/Picking/Picking.h"
#include "Render/Viewport/Viewport.h"
#include <unordered_set> 
#include <memory>
#include "App/Snap/SnapEngine.h"
#include "App/Snap/SnapResult.h"
namespace MiniCAD
{
	class Editor  
	{
	public:
		Editor(Scene& scene, CommandStack& cmdStack, Viewport& viewport, Overlay& overlay, Picking& picking, SnapEngine& snap, SnapResult& currentSnap);
		bool OnInput(const InputEvent& e);

		// Picking 获取选择 
		const std::unordered_set<Object::ObjectID>& GetSelection();
		const std::unordered_set<Object::ObjectID>& GetHovered();
		const bool IsAcitveTool() { return m_tool != nullptr; };  

	private:
		bool HandleGlobal(const InputEvent& e);
		bool HandleDefault(const InputEvent& e);
		void StartLineTool();
	private:
		bool          ShouldSnap() const;                   // 允许捕获 
		void          UpdateSnap(const InputEvent& e);      // 更新捕获点currentSnap
		InputEvent    InjectSnap(const InputEvent& e);      // 注入捕获点
	private:
		Scene&        m_scene;            
		CommandStack& m_cmdStack;		   
		Viewport&     m_viewport;		   
		Overlay&      m_overlay;

		Picking&      m_picking; 
		std::unique_ptr<ITool>   m_tool;

		SnapEngine&   m_snap;
		SnapResult&   m_currentSnap;
	};
}