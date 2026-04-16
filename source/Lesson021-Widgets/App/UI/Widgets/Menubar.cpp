#include "Menubar.h"
#include <imgui.h>
#include "App/Document/Document.h"
namespace MiniCAD
{
    const char* Menubar::GetName() const
    {
        return "Menubar";
    }

    void Menubar::OnRender(Document& document)
    {
        // 不能用 Begin
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
    }
}