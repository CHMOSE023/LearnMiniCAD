#include "Editor.h"
#include "App/Tools/LineTool.h"
#include "App/CommandStack/CommandStack.h"
#include "App/Scene/Scene.h" 
#include "Render/Viewport/Viewport.h"
#include <memory>
namespace MiniCAD
{
	Editor::Editor(Scene& scene, CommandStack& cmdStack, Viewport& viewport)
		: m_scene(scene)
		, m_cmdStack(cmdStack)
		, m_viewport(viewport)
	{ 
	}

	bool Editor::OnInput(const InputEvent& e)
	{ 
		// 1. 全局
		if (HandleGlobal(e)) return true;

		// 2. Tool
		if (m_tool && m_tool->OnInput(e)) return true;

		// 3. 默认
		return HandleDefault(e);

	}

   
    bool Editor::HandleGlobal(const InputEvent& e)
    { 
        if (e.IsUndo())  // Ctrl + Z  撤销
        {
            m_cmdStack.Undo(m_scene);
            return true;
        }

        if (e.IsRedo())  // Ctrl + Z 重做
        {
            m_cmdStack.Redo(m_scene);
            return true;

        }

        if (e.IsCancel()) // 取消
        {
            if (m_tool)
            {
                m_tool->Cancel();
                m_tool.reset();
            }
            return true;
        }

        if (e.IsStartLineTool()) // 绘制直线
        {
            StartLineTool();
            return true;
        }

        switch (e.Type)
        {
        case InputEventType::MouseMove:
            if (e.IsMouseButtonDown(MouseButton::Middle))
            {
                m_viewport.Pan(e.MouseX - e.LastMouseX, e.MouseY - e.LastMouseY);
                return true;
            }
            break;

        case InputEventType::MouseWheel:
            m_viewport.Zoom(e.WheelDelta, e.MouseX, e.MouseY);
            return true;
        }
        return false;
    }

    bool Editor::HandleDefault(const InputEvent& e)
    { 
        return false;
    }

	void Editor::StartLineTool()
	{
		if (m_tool)
			m_tool->Cancel(); // 自动退出旧工具

		m_tool = std::make_unique<LineTool>(m_scene, m_cmdStack, m_viewport);
		m_tool->OnFinished = [this]() {m_tool.reset(); }; // 内部结束绘制
		printf("[Editor] Start LineTool\n");
	}


}