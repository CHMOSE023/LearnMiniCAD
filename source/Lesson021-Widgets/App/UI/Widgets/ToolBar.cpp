#include "ToolBar.h"
#include <imgui.h> 
#include "App/Document/Document.h"
#include <imgui_internal.h>
namespace MiniCAD
{
    const char* ToolBar::GetName() const
    {
        return "工具栏";
    }

    void ToolBar::OnRender(Document& document)
    { 
        ImGui::Begin("##Toolbar", nullptr );
        {
            auto& editor = document.GetEditor();
           
            if (ImGui::Button("直线", ImVec2(60, 32)))
            {
                editor.StartLineTool();
            }
 
        }
        ImGui::End();
    }
}