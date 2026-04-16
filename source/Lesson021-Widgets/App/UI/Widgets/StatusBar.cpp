#include "StatusBar.h"
#include <imgui.h> 

#include "App/Document/Document.h"
namespace MiniCAD
{
    const char* StatusBar::GetName() const
    {
        return "状态";
    }
      
    static const ImVec4 COLOR_ACTIVE   = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);  // 蓝色：激活
    static const ImVec4 COLOR_INACTIVE = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);  // 灰色：未激活

    void StatusBar::OnRender(Document& document)
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + vp->Size.y - 25));
        ImGui::SetNextWindowSize(ImVec2(vp->Size.x, 25));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

        if (!ImGui::Begin(GetName(), nullptr, flags))
        {
            ImGui::End();
            return;
        } 
        float lh = ImGui::GetTextLineHeight();
        float h  = ImGui::GetWindowHeight();
        ImGui::SetCursorPosX(10.0);
        ImGui::SetCursorPosY(5.0);

        auto viesState = document.GetEditor().BuildViewState();

        auto world = document.GetEditor().GetScene()->GetCamera()->ScreenToWorld(viesState.MouseX, viesState.MouseY);
          
        // 当前坐标状态
        ImGui::BeginChild("status_world", ImVec2(180, 0), false);
        ImGui::Text(std::format("{:.4f},   {:.4f},   {:.4f}", world.x, world.y,world.z).c_str());
        ImGui::EndChild();

        ImGui::SameLine(); 

       
        // 捕捉状态
        ImGui::BeginChild("status_snap", ImVec2(80, 0), false);
        {
            bool snapEnabled = document.GetEditor().IsSnapEnabled();
            ImVec2 btnPos = ImGui::GetCursorPos();
            ImGui::TextColored(snapEnabled ? COLOR_ACTIVE : COLOR_INACTIVE, "捕捉: ");
            ImGui::SameLine();
            ImGui::TextColored(snapEnabled ? COLOR_ACTIVE : COLOR_INACTIVE, snapEnabled ? "开 " : "关  ");
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
            ImGui::TextColored(orthoEnabled ? COLOR_ACTIVE : COLOR_INACTIVE, "正交: ");
            ImGui::SameLine();
            ImGui::TextColored(orthoEnabled ? COLOR_ACTIVE : COLOR_INACTIVE, orthoEnabled ? "开 " : "关 ");
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