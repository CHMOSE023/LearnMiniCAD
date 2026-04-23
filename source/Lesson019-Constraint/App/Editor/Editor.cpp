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
    Editor::Editor(Scene& scene, CommandStack& cmdStack, Viewport& viewport,
        Overlay& overlay, Picking& picking,
        SnapEngine& snap, SnapResult& currentSnap)
        : m_scene(scene)
        , m_cmdStack(cmdStack)
        , m_viewport(viewport)
        , m_overlay(overlay)
        , m_picking(picking)
        , m_snap(snap)
        , m_currentSnap(currentSnap)
        , m_gripEditor(m_viewport, m_scene, m_cmdStack, m_picking) 
        , m_anchorLine({}, {})
    {
    }

    bool Editor::OnInput(const InputEvent& inputEvent)
    {
        UpdateSnap(inputEvent);
        InputEvent e = InjectSnap(inputEvent);

        // 1. 全局快捷键（Undo / Redo / Cancel / 工具切换 / 视图操作）
        if (HandleGlobal(e))
            return true;

        // 获取约束修改后的 事件。
        e = ApplyConstraints(e);

        // 2. Tool 激活时完全屏蔽后续系统
        if (m_tool)
            return m_tool->OnInput(e);

        // 3. 夹点拖拽编辑 
        if (m_gripEditor.OnInput(e))
            return true;
         
        // 4. Picking（选择系统） 
        if (!m_gripEditor.IsDragging()) //拖拽进行中完全跳过：避免 hover / selection 被误改，
        {  
            if (m_picking.OnInput(e))
            {  
                m_gripEditor.MarkDirty();   
                return true;
            }

        }

        // 5. 默认处理
        return HandleDefault(e);
    }

    // ─────────────────────────────────────────────
    //  HandleGlobal
    // ─────────────────────────────────────────────
    bool Editor::HandleGlobal(const InputEvent& e)
    {
        if (e.IsUndo())
        {
            m_cmdStack.Undo(m_scene);
            m_gripEditor.MarkDirty();  // 线段数据已变，夹点需要重建
            return true;
        }

        if (e.IsRedo())
        {
            m_cmdStack.Redo(m_scene);
            m_gripEditor.MarkDirty();  // 同上
            return true;
        }

        if (e.IsCancel())
        {
            if (m_tool)
            {
                m_tool->Cancel();
                m_tool.reset();
                m_overlay.Clear();
            }
            else if (m_gripEditor.IsDragging())  // 取消拖动恢复样式
            {
                m_gripEditor.CancelDrag();
                m_gripEditor.MarkDirty();   
            }
            return true;
        }

        if (e.IsStartLineTool())
        {
            StartLineTool();
            return true;
        }
        
        if (e.Type == InputEventType::KeyDown && e.KeyCode == VK_F8)  // F8 启用或关闭正交
        {
            m_orthoEnabled = !m_orthoEnabled;

            printf("[Editor] Ortho: %s\n", m_orthoEnabled ? "ON" : "OFF");

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

        default:
            break;
        }

        return false;
    }

    bool Editor::HandleDefault(const InputEvent& /*e*/)
    {
        return false;
    }

    // ─────────────────────────────────────────────
    //  StartLineTool
    // ─────────────────────────────────────────────
    void Editor::StartLineTool()
    {
        if (m_tool)
            m_tool->Cancel();

        m_picking.ClearSelection();

        m_tool = std::make_unique<LineTool>(m_scene, m_cmdStack, m_viewport, m_overlay);
        m_tool->OnFinished = [this]()
            {
                m_overlay.Clear();
                m_tool.reset();
            };

        printf("[Editor] Start LineTool\n");
    }

    // ─────────────────────────────────────────────
    //  Snap
    // ─────────────────────────────────────────────
    void Editor::UpdateSnap(const InputEvent& e)
    {
        if (!ShouldSnap())
            return;

        const Camera& cam = m_viewport.GetCamera();

        // 夹点拖拽时排除选中对象，避免捕捉到自身端点
        const auto& exclude = m_gripEditor.IsDragging() ? m_picking.GetSelection() : std::unordered_set<Object::ObjectID>{};

        switch (e.Type)
        {
        case InputEventType::MouseMove:
        case InputEventType::MouseButtonDown:
        case InputEventType::MouseButtonUp:
            m_currentSnap = m_snap.Query( { static_cast<float>(e.MouseX), static_cast<float>(e.MouseY) },m_scene, cam, exclude);
            break;
        default:
            break;
        }
    }

    bool Editor::ShouldSnap() const
    {
        if (m_tool) return true;                        // 绘制工具激活时捕捉
        if (m_gripEditor.IsDragging()) return true;     // 夹点拖拽时也捕捉
        return false;
    }

    InputEvent Editor::InjectSnap(const InputEvent& e)
    {
        InputEvent out = e;
        out.HasSnap    = false; 

        if (m_currentSnap.IsValid())
        { 
            out.HasSnap = true;
            out.SnapWorld = m_currentSnap.WorldPos;
        }

        return out;
    }

    InputEvent Editor::ApplyConstraints(const InputEvent& e)
    {
        InputEvent out = e;

        if (!m_orthoEnabled)
        {
            m_anchorLine = { {},{} };  // 清空约束辅助线
            return out;
        }

        DirectX::XMFLOAT3 anchor;

        if (!TryGetAnchor(anchor))     // 获取约束点
        {
            m_anchorLine = { {},{} };  // 清空约束辅助线
            return out;
        }

        DirectX::XMFLOAT3 input;

        if (e.HasSnap)
        {
            input = e.SnapWorld;
        }
        else
        {
            auto p = m_viewport.GetCamera().ScreenToWorld(e.MouseX, e.MouseY);
            input = DirectX::XMFLOAT3(p.x, p.y, 0.f);
        }

        float dx = input.x - anchor.x;
        float dy = input.y - anchor.y;

        DirectX::XMFLOAT3 result;

        if (fabs(dx) > fabs(dy))
            result = { input.x, anchor.y, 0.f };
        else
            result = { anchor.x, input.y, 0.f };

        out.HasSnap = true;
        out.SnapWorld = result;

		m_anchorLine = { anchor, result }; // 更新约束辅助线
      
        return out;

    }

    bool Editor::TryGetAnchor(DirectX::XMFLOAT3& out) const
    {
        if (m_tool && m_tool->HasAnchor()) // 锚点，约束点
        {
            out = m_tool->GetAnchor();
            return true;
        }

        if (m_gripEditor.IsDragging())
        {
            out = m_gripEditor.GetDragBase();
            return true;
        }

        return false;
    }

}  
