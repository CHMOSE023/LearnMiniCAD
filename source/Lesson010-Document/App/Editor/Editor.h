#pragma once
#include "App/Input/IInputHandler.h" 
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h"
namespace MiniCAD
{
	class Editor //:public IInputHandler
	{
	public:
		Editor(Scene& scene, CommandStack& cmdStack);
		bool OnInput(const InputEvent& e);

	private:
		void OnMouseButtonDown(const InputEvent& e);
		void OnMouseButtonUp(const InputEvent& e);
		void OnKeyDown(const InputEvent& e);
		void OnKeyUp(const InputEvent& e);
		void OnMouseMove(const InputEvent& e);
		void OnMouseWheel(const InputEvent& e);

	private:
		Scene&        m_scene;
		CommandStack& m_cmdStack;
	};
}