#include "UIManager.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>
#include "Widgets/PropertyPanel.h"
#include "Widgets/Menubar.h"
#include "Widgets/StatusBar.h"
#include "Widgets/ToolBar.h"
#include <memory>
namespace MiniCAD
{
    bool UIManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_imgui = std::make_unique<ImGuiLayer>();

        if (!m_imgui->Init(hwnd, device, context))
            return false;
         
        m_widgets.push_back(std::make_unique<PropertyPanel>());  // 添加属性面板
        m_widgets.push_back(std::make_unique<Menubar>());        // 菜单栏
        m_widgets.push_back(std::make_unique<StatusBar>());      // 状态栏
        m_widgets.push_back(std::make_unique<ToolBar>());        // 工具栏


        // 启用 Docking（关键）
        ImGuiIO& io = ImGui::GetIO(); 

        io.Fonts->AddFontFromFileTTF(
            "C:/Windows/Fonts/msyh.ttc",   // 微软雅黑
            16.0f,
            nullptr,
            io.Fonts->GetGlyphRangesChineseFull()
        );

        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.ChildRounding = 0.0f;

        style.WindowPadding = ImVec2(0, 0);
        style.FramePadding = ImVec2(4, 2);
        style.ItemSpacing = ImVec2(4, 4);

        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 0.0f;
        style.FrameBorderSize = 0.0f;

        ImGui::StyleColorsLight(); // 黑色
        return true;
    }

    void UIManager::Shutdown()
    {
        m_imgui->Shutdown();
    }

    void UIManager::BeginFrame()
    {
        m_imgui->Begin();
    }

    void UIManager::EndFrame()
    {
        m_imgui->End();
    }

    // =========================================================
    // 主渲染入口 
    // =========================================================
    void UIManager::Render(Document& document)
    {
        DrawDockSpace(document);           // 框架  

        for (const auto& widget : m_widgets)
        {
            if (!widget) continue;
            widget->OnRender(document);   // 传进去
        }
       
    }

    // =========================================================
    // DockSpace 
    void UIManager::DrawDockSpace(Document& document)
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(vp->WorkPos);  // 去掉 MenuBar
        ImGui::SetNextWindowSize(vp->WorkSize);// 去掉 MenuBar
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        ImGui::Begin("DockRoot", nullptr, flags);

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

        ImGui::DockSpace(dockspace_id, ImVec2(0, 0),
            ImGuiDockNodeFlags_PassthruCentralNode);

        // 只执行一次布局
        static bool init = true;
        if (init)
        {
            init = false;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, vp->Size);

            ImGuiID dock_main = dockspace_id;

            ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.2f, nullptr, &dock_main);

            ImGui::DockBuilderDockWindow("状态", dock_bottom);
            ImGui::DockBuilderDockWindow("命令", dock_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::End();
    } 

}