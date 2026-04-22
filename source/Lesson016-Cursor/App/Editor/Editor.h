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
namespace MiniCAD
{
	class Editor  
	{
	public:
		Editor(Scene& scene, CommandStack& cmdStack, Viewport& viewport, Overlay& overlay, Picking& picking);
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
		Scene&        m_scene;            
		CommandStack& m_cmdStack;		   
		Viewport&     m_viewport;		   
		Overlay&      m_overlay;

		Picking&      m_picking; 
		std::unique_ptr<ITool>   m_tool;
	};
}