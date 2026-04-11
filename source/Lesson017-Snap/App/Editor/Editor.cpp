#include "Editor.h"
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h"  
#include "App/Input/InputEvent.h" 
#include "App/Editor/Tools/LineTool.h"
#include "App/Command/DeleteEntityCommand.h"
#include "App/Command/BatchDeleteCommand.h"
#include <memory>
namespace MiniCAD
{
    Editor::Editor(Scene* scene, CommandStack* cmdStack)
        : m_scene(scene)
        , m_cmdStack(cmdStack)
		, m_picking(scene)
		, m_gripEditor(scene, &m_gripManager)
    {
    }

    /// <summary>
    /// 责任链模式，消息处理有优先级
    /// </summary> 
    bool Editor::OnInput(const InputEvent& e)
    { 
        // 1️ 全局按键优先处理
        if (e.Type == InputEventType::KeyDown && e.KeyCode == VK_ESCAPE)
        { 
            if (m_tool)
            {
                m_tool->Cancel();
                m_tool.reset();
                printf("[Editor] ESC exit tool\n");
                return true;
            }
        }

        // 2️ 工具处理
        if (m_tool)
        {
            switch (e.Type)
            {
            case InputEventType::MouseButtonDown: m_tool->OnMouseDown(e); return true;
            case InputEventType::MouseButtonUp:   m_tool->OnMouseUp(e);   return true;
            case InputEventType::MouseMove:       m_tool->OnMouseMove(e); OnMouseMove(e); return true; 
            case InputEventType::KeyDown:         m_tool->OnKeyDown(e);   return true;
            case InputEventType::MouseWheel:      OnMouseWheel(e);        return true;
            default: return false;
            } 
        }

        // 3️ 默认 Editor 自己处理
        switch (e.Type)
        {
        case InputEventType::MouseButtonDown: OnMouseButtonDown(e); return true;
        case InputEventType::MouseButtonUp:   OnMouseButtonUp(e);   return true;
        case InputEventType::MouseWheel:      OnMouseWheel(e);      return true;
        case InputEventType::MouseMove:       OnMouseMove(e);       return true;
        case InputEventType::KeyDown:         OnKeyDown(e);         return true;
        default:
            return false;
        } 
        return false;
    }

    void Editor::OnResize(float width, float height)
    {
        m_scene->GetCamera()->Resize(width, height);
    }

	// 帧度更新时，Viewport 会调用 Editor::BuildViewState 
    // 来获取当前的 ViewState（Selection/Hovered/ShowGrid/ShowGizmo）
    ViewState Editor::BuildViewState() const
    {
        ViewState vs;

        vs.ShowGrid     = m_showGrid;
        vs.ShowGizmo    = m_showGizmo;
		vs.MouseX       = static_cast<float>(m_mouseX);
		vs.MouseY       = static_cast<float>(m_mouseY);
                        
        vs.Selection    = &m_picking.GetSelection();
        vs.Hovered      = &m_picking.GetHovered();

        vs.BoxSelected  = m_picking.IsBoxSelected();
        vs.BoxPressX    = m_picking.GetBoxPress().x;
        vs.BoxPressY    = m_picking.GetBoxPress().y;

        auto* cam = m_scene->GetCamera();

        if (m_showGizmo)
        { 
            auto hoveredIdx = m_gripEditor.HoveredGrip();
         
            for (int i = 0; i < (int)m_gripManager.GetGrips().size(); ++i)
            {
                const auto& g = m_gripManager.GetGrips()[i];

                auto s = cam->WorldToScreen(g.WorldPos);

                auto type = static_cast<GripDraw::Type>(g.GripType);

                bool hovered = (i == hoveredIdx);
               
                vs.Grips.push_back({ s, type,hovered });
            }
        }

        vs.Snap.Pos       = cam->WorldToScreen(m_currentSnap.WorldPos);
        vs.Snap.SnapType  = static_cast<SnapDraw::Type>(m_currentSnap.SnapType); 
        return vs;
    }

    const std::unordered_set<Object::ObjectID>& Editor::GetSelection()
    {
		return m_picking.GetSelection();
    }
    
    const std::unordered_set<Object::ObjectID>& Editor::GetHovered()
    {
        return m_picking.GetHovered();
    }


    void Editor::OnKeyDown(const InputEvent& e)
    {  
        m_picking.OnKeyDown(e);

        if (e.KeyCode == VK_ESCAPE)
        {
            OnSelectionChanged();   // ESC 清空选集，同步夹点
        }

        if (e.KeyCode == 'L')
        { 
            m_tool = std::make_unique<LineTool>(m_scene, m_cmdStack);
            return;
        }
		// Delete：删除选中
        if (e.KeyCode == VK_DELETE)
        { 
            DeleteSelected();
            return;
        }

        // Ctrl+Z：Undo
        if (e.HasModifier(ModifierKey::Ctrl) && e.KeyCode == 'Z')
        {
            m_cmdStack->Undo(*m_scene); 
            return;
        } 
        // Ctrl+Y：Redo
        if (e.HasModifier(ModifierKey::Ctrl) && e.KeyCode == 'Y')
        {
            m_cmdStack->Redo(*m_scene); 
            return;
        } 
		
        // 释放命令
        if (m_tool&& e.KeyCode == VK_ESCAPE)
        {
            m_tool->Cancel();
            m_tool.reset();
            printf("[Editor] ESC exit tool\n");
            return ;
        }
         
      
    }

    void Editor::OnKeyUp(const InputEvent& e)
    {
    }

    void Editor::OnMouseButtonUp(const InputEvent& e)
    {
        if (m_gripEditor.OnMouseUp(e))
        {
            m_gripManager.Rebuild(m_picking.GetSelection(), m_scene); // 夹点拖拽结束 → 几何已变，重建夹点
            return;
        }

        auto selBefore = m_picking.GetSelection();

        m_picking.OnMouseUp(e);

        if (m_picking.ConsumeSelectionChanged()) // 选集是否变化
            OnSelectionChanged();
        
    }

    void Editor::OnMouseButtonDown(const InputEvent& e)
    {

        if (m_gripEditor.OnMouseDown(e)) return;

        m_picking.OnMouseDown(e); 

        auto world =  m_scene->GetCamera()->ScreenToWorld(e.MouseX,e.MouseY);
        XMFLOAT2 screen = m_scene->GetCamera()->WorldToScreen(world);
	    printf("[Editor] MouseDown at screen (%0.2f, %0.2f), world (%.2f, %.2f)\n", screen.x, screen.y, world.x, world.y);

    }
    void Editor::OnMouseMove(const InputEvent& e)
    { 
        m_mouseX = e.MouseX;
        m_mouseY = e.MouseY;

        if (e.IsMouseButtonDown(MouseButton::Middle)) // 平移相机
        { 
            m_scene->GetCamera()->Pan(e.MouseX - e.LastMouseX, e.MouseY - e.LastMouseY);
        }
        
		//  每帧统一算一次，结果缓存在 m_currentSnap
        const Camera* cam = m_scene->GetCamera();
        m_currentSnap = m_snap.Query({ m_mouseX, m_mouseY }, m_scene, cam);
 
        if (m_currentSnap.IsValid())
        {
            printf("[Editor]  CurrentSnap  (%0.2f, %0.2f) \n", m_currentSnap.WorldPos.x, m_currentSnap.WorldPos.y);
        }
		
      
        
        if (m_gripEditor.OnMouseMove(e))
        {
			m_gripManager.Rebuild(m_picking.GetSelection(), m_scene); // 夹点拖拽中 → 几何变了，实时重建夹点
            return;
        }

        m_picking.OnMouseMove(e);        
    }

	// 鼠标滚轮：缩放
    void Editor::OnMouseWheel(const InputEvent& e)
    {
		m_scene->GetCamera()->Zoom(e.WheelDelta, e.MouseX, e.MouseY);
    }

    void Editor::DeleteSelected()
    {
		auto ids = m_picking.GetSelection(); // 获取选中对象 ID 列表
        if (ids.empty())
            return;

		std::vector<Object::ObjectID> idsVec(ids.begin(), ids.end()); // 转换为 vector

        auto cmd = std::make_unique<BatchDeleteCommand>(idsVec);

        m_cmdStack->Execute(std::move(cmd), *m_scene); 
      
    }

    // ─── 选集变化 ─────────────────────────────────────────────────────────────────
    void Editor::OnSelectionChanged()
    {
        m_gripManager.Rebuild(m_picking.GetSelection(), m_scene);
    }
     
   
}
