#include "Editor.h"
#include "App/Tools/LineTool.h"
#include "App/CommandStack/CommandStack.h"
#include "App/Scene/Scene.h" 
#include "App/Overlay/Overlay.h"
#include "App/Picking/Picking.h"
#include "Render/Viewport/Viewport.h"
#include "Render/Viewport/Camera.h"
#include <memory>
#include <cstdio>
namespace MiniCAD
{
    Editor::Editor(Scene& scene, CommandStack& cmdStack, Viewport& viewport, Overlay& overlay, Picking& picking, SnapEngine& snap, SnapResult& currentSnap)
        : m_scene(scene)
        , m_cmdStack(cmdStack)
        , m_viewport(viewport)
        , m_overlay(overlay)
        , m_picking(picking)
        , m_snap(snap)
        , m_currentSnap(currentSnap)
	{ 
	}

	bool Editor::OnInput(const InputEvent& inputEvent)
	{ 
        UpdateSnap(inputEvent);                // 更新捕获
        InputEvent e = InjectSnap(inputEvent); // 注入捕获

		// 1. 全局
		if (HandleGlobal(e)) return true;
         
		// 2. Tool
        if (m_tool)
        { 
            return m_tool->OnInput(e);  // Tool 激活 → 完全屏蔽 Picking
        }

        // 3. Picking（选择系统）
        if (m_picking.OnInput(e)) return true;

		// 4. 默认
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
                m_overlay.Clear(); // 清空预览
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

        m_picking.ClearSelection(); // 清空选择

        m_tool = std::make_unique<LineTool>(m_scene, m_cmdStack, m_viewport, m_overlay);
		m_tool->OnFinished = [this]() { 
            m_overlay.Clear(); // 清空预览
            m_tool.reset(); 
        }; // 内部结束绘制
		printf("[Editor] Start LineTool\n");
	}

    void Editor::UpdateSnap(const InputEvent& e)
    {
        if (!ShouldSnap())
            return; 

        const Camera& cam = m_viewport.GetCamera(); 
      
        switch (e.Type)  // 只在鼠标相关事件时更新（避免无意义计算）
        {
        case InputEventType::MouseMove:
        case InputEventType::MouseButtonDown:
        case InputEventType::MouseButtonUp:
            m_currentSnap = m_snap.Query({ static_cast<float>(e.MouseX), static_cast<float>(e.MouseY) }, m_scene, cam);
            break;
        default:
            break;
        }
    }

    bool Editor::ShouldSnap() const
    {
        if (m_tool) return true; // 开始绘制时候
      
        return false;
    }

    InputEvent Editor::InjectSnap(const InputEvent& e)
    {
        InputEvent out = e;  
        out.HasSnap = false;
        if (m_currentSnap.IsValid())
        {
            printf("m_currentSnap %0.3f ,%0.3f \n", m_currentSnap.WorldPos.x , m_currentSnap.WorldPos.y);
            out.HasSnap = true;
            out.SnapWorld = m_currentSnap.WorldPos;
        }  

        return out;
    }
     
}