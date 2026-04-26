#include "UIManager.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h> 
#include <memory>
#include "App/Document/Document.h"
#include "Widgets/Menubar.h"
#include "Widgets/PropertyPanel.h"
#include "Widgets/StatusBar.h"
#include "Widgets/ToolBar.h"
#include "Widgets/ToolBar.h"
#include "Widgets/LayerManagerWidget.h"
namespace MiniCAD
{
    bool UIManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_imgui = std::make_unique<ImGuiLayer>();
        if (!m_imgui->Init(hwnd, device, context))
            return false;
          
        m_widgets.push_back(std::make_unique<PropertyPanel>());      // 添加属性面板
        m_widgets.push_back(std::make_unique<Menubar>());            // 菜单栏
        m_widgets.push_back(std::make_unique<StatusBar>());          // 状态栏
        m_widgets.push_back(std::make_unique<ToolBar>());            // 工具栏 
        m_widgets.push_back(std::make_unique<LayerManagerWidget>()); // 图层管理

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF(
            "C:/Windows/Fonts/msyh.ttc",
            16.0f,
            nullptr,
            io.Fonts->GetGlyphRangesChineseFull()
        );

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding   = 4.0f;
        style.FrameRounding    = 3.0f;
        style.ChildRounding    = 3.0f;
        style.WindowPadding    = ImVec2(6, 6);  // 不能为 (0,0)，否则标题栏拖拽失效
        style.FramePadding     = ImVec2(4, 2);
        style.ItemSpacing      = ImVec2(4, 4);
        style.WindowBorderSize = 1.0f;          // 保留，停靠预览高亮边框需要
        style.ChildBorderSize  = 1.0f;
        style.FrameBorderSize  = 0.0f;

         // ImGui::StyleColorsLight();          // 明亮风格
         ImGui::StyleColorsClassic();
          // ImGui::StyleColorsDark();
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
         DrawDockSpace(document);

        for (const auto& widget : m_widgets)
        {
            if (!widget) continue;
            widget->OnRender(document);
        }

      
    }

    // =========================================================
    // DockSpace
    // =========================================================
    void UIManager::DrawDockSpace(Document& document)
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        // DockRoot 自己用 (0,0) padding，不影响其他子窗口
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("DockRoot", nullptr, flags);
        ImGui::PopStyleVar();

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0),
            ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();
    }


}