#include "StatusBar.h"
#include <imgui.h> 
#include <format>
#include "App/Document/Document.h"
namespace MiniCAD
{
    const char* StatusBar::GetName() const
    {
        return "状态栏";
    } 

    StatusBar::StatusBar()
    {
        m_id = "status_bar";
    }

    void StatusBar::OnRender(Document& document)
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - 25));
        ImGui::SetNextWindowSize(ImVec2(vp->Size.x, 25));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                                 ImGuiWindowFlags_NoResize   | 
                                 ImGuiWindowFlags_NoMove     | 
                                 ImGuiWindowFlags_NoScrollbar;

        if (!ImGui::Begin(GetName(), nullptr, flags))
        {
            ImGui::End();
            return;
        } 

        auto& style = ImGui::GetStyle(); 

        // 使用 ImGui 语义颜色（自动适配明暗主题）
        const ImVec4 colorActive   = style.Colors[ImGuiCol_Text];
        const ImVec4 colorInactive = style.Colors[ImGuiCol_TextDisabled];
         
        ImGui::SetCursorPosX(10.0);
        ImGui::SetCursorPosY(5.0);

        auto viesState = document.BuildViewState();
         
        // 当前坐标状态
        ImGui::BeginChild("status_world", ImVec2(180, 0), false);
        {
            if (m_worldActive)
            {
                m_worldFrozen = document.GetViewport().GetCamera().ScreenToWorld(viesState.MouseX, viesState.MouseY);
            }

            ImGui::TextColored(
                m_worldActive ? colorActive : colorInactive,
                "%.4f, %.4f, %.4f",
                m_worldFrozen.x,
                m_worldFrozen.y,
                m_worldFrozen.z
            );

            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("world_toggle", ImGui::GetContentRegionAvail());

            if (ImGui::IsItemClicked())
            {
                m_worldActive = !m_worldActive;
            } 
        }
        ImGui::EndChild();


        ImGui::SameLine(); 

       
        // 捕捉状态
        ImGui::BeginChild("status_snap", ImVec2(80, 0), false);
        {
            bool snapEnabled = document.GetEditor().IsSnapEnabled();
            ImVec2 btnPos = ImGui::GetCursorPos();
            ImGui::TextColored(snapEnabled ? colorActive : colorInactive, "捕捉(F3): ");
            ImGui::SameLine();
            ImGui::TextColored(snapEnabled ? colorActive : colorInactive, snapEnabled ? "开 " : "关  ");
            ImGui::SetCursorPos(btnPos);
            ImGui::InvisibleButton("snap_toggle", ImVec2(80, ImGui::GetTextLineHeight()));
            if (ImGui::IsItemClicked())
            {
                document.GetEditor().ToggleSnap();
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // 正交状态
        ImGui::BeginChild("status_ortho", ImVec2(80, 0), false);
        {
            bool orthoEnabled = document.GetEditor().IsOrthoEnabled();

            ImVec2 btnPos = ImGui::GetCursorPos();
            ImGui::TextColored(orthoEnabled ? colorActive : colorInactive, "正交(F8): ");
            ImGui::SameLine();
            ImGui::TextColored(orthoEnabled ? colorActive : colorInactive, orthoEnabled ? "开 " : "关 ");
            ImGui::SetCursorPos(btnPos);
            ImGui::InvisibleButton("ortho_toggle", ImVec2(80, ImGui::GetTextLineHeight()));
            if (ImGui::IsItemClicked())
            {
                document.GetEditor().ToggleOrtho();
            }
        }
        ImGui::EndChild();
        

        ImGui::End();
         
    }
}