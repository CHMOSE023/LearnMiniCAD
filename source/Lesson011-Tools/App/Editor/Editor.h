#pragma once
#include "App/Input/IInputHandler.h" 
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h" 
#include "App/Tools/ITool.h"
#include "Render/Viewport/Viewport.h"
#include <memory>
namespace MiniCAD
{
	class Editor  
	{
	public:
		Editor(Scene& scene, CommandStack& cmdStack, Viewport& viewport);
		bool OnInput(const InputEvent& e);

	private:
		bool HandleGlobal(const InputEvent& e);
		bool HandleDefault(const InputEvent& e);
		void StartLineTool();
	private:
		Scene&        m_scene;            
		CommandStack& m_cmdStack;		   
		Viewport&     m_viewport;		   

		std::unique_ptr<ITool>   m_tool;
	};
}