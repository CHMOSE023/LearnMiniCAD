#include "Editor.h"
namespace MiniCAD
{
	Editor::Editor(Scene* scene, CommandStack* cmdStack)
	{
		m_scene = scene;
		m_cmdStack = cmdStack;
	}

	bool Editor::OnInput(const InputEvent& e)
	{
		switch (e.Type)
		{
		case InputEventType::MouseButtonDown:
			OnMouseButtonDown(e);
			return true;
		case InputEventType::MouseButtonUp:
			OnMouseButtonUp(e);
			return true;
		case InputEventType::MouseWheel:
			OnMouseWheel(e);
			return true;
		case InputEventType::MouseMove:
			OnMouseMove(e);
			return true;
		case InputEventType::KeyDown:
			OnKeyDown(e);
			return true;
		default:
			return false;
		}
		return false; 
	}


	void Editor::OnMouseButtonDown(const InputEvent& e)
	{

	}

	void Editor::OnMouseButtonUp(const InputEvent& e)
	{

	}

	void Editor::OnKeyDown(const InputEvent& e)
	{  
		// Ctrl+Z：Undo
		if (e.HasModifier(ModifierKey::Ctrl) && e.KeyCode == 'Z')
		{
			printf("Ctrl+Z -> Undo\n");
			m_cmdStack->Undo(*m_scene);
			return;
		}
		// Ctrl+Y：Redo
		if (e.HasModifier(ModifierKey::Ctrl) && e.KeyCode == 'Y')
		{
			printf("Ctrl+Y -> Redo\n");
			m_cmdStack->Redo(*m_scene);
			return;
		}

	}

	void Editor::OnKeyUp(const InputEvent& e)
	{

	}

	void Editor::OnMouseMove(const InputEvent& e)
	{

	}

	void Editor::OnMouseWheel(const InputEvent& e)
	{

	}


}