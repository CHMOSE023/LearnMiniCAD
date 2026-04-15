#include "UIManager.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

namespace MiniCAD
{
    bool UIManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_imgui = std::make_unique<ImGuiLayer>();

        if (!m_imgui->Init(hwnd, device, context))
            return false;

        // 启用 Docking（关键）
        ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
    void UIManager::Render(MiniCAD::Editor& editor)
    {
        DrawDockSpace(editor);   // 框架
        DrawPanels(editor);      // 面板
    }

    // =========================================================
    // DockSpace 
    void UIManager::DrawDockSpace(MiniCAD::Editor& editor)
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
    // =========================================================
    // Panels
    // =========================================================
    void UIManager::DrawPanels(MiniCAD::Editor& editor)
    {
        // ===== 顶部菜单 =====
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("文件"))
            {
                ImGui::MenuItem("新建");
                ImGui::MenuItem("打开");
                ImGui::MenuItem("保存");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("编辑"))
            {
                ImGui::MenuItem("撤销");
                ImGui::MenuItem("重做");
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // ===== 左：属性 =====
        ImGui::Begin("属性");
        {
            ImGui::Text("No Selection");
        }
        ImGui::End();

        // ===== 右：工具 =====
   /*     ImGui::Begin("工具");
        {
            if (ImGui::Button("Line")) {}
            if (ImGui::Button("Move")) {}
            if (ImGui::Button("Trim")) {}
        }
        ImGui::End();*/

        // ===== 中：CAD视口（核心）=====
        //ImGui::Begin("");
        //{ 
        //    ImVec2 size = ImGui::GetContentRegionAvail();

        //    // 这里后面替换成你的 D3D 渲染 SRV
        //    // ImGui::Image((ImTextureID)m_viewportSRV, size);

        //    ImGui::Text("CAD Viewport");
        //    ImGui::Text("Size: %.0f x %.0f", size.x, size.y);
        //}
        //ImGui::End();

        // ===== 底部：命令行 =====
        ImGui::Begin("命令");
        {
            static char cmd[256]{};

            if (ImGui::InputText("##cmd", cmd, sizeof(cmd),
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                cmd[0] = 0;
            }
        }
        ImGui::End();

        // ===== 固定在底部状态栏 =====
      /*  ImGui::Begin("状态");
        {
            ImGui::Text("Tool: %s", "editor.GetActiveToolName().c_str()");
            ImGui::Text("Entities: %s", "editor.GetEntityCount()");
        }
        ImGui::End();*/
    }

}