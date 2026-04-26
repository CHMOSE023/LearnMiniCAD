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
                ImGui::MenuItem("新建", "Ctrl+N",false,false); {}
                ImGui::MenuItem("打开", "Ctrl+O", false, false); {}
                if (ImGui::BeginMenu("最近打开",false))
                {
                    ImGui::MenuItem("Hello");
                    ImGui::MenuItem("Sailor");
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("保存", "Ctrl+S", false, false)) {}
                if (ImGui::MenuItem("另存为..", "Ctrl+S", false, false)) {}
                ImGui::Separator();
                if (ImGui::MenuItem("导入..", "Ctrl+I", false, false)) {}
                if (ImGui::MenuItem("导出..", "Ctrl+E", false, false)) {}
                ImGui::Separator();
                if (ImGui::MenuItem("退出", "Alt+F4", false, false)) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("编辑"))
            {
                if (ImGui::MenuItem("撤销", "Ctrl+Z")) { document.Undo(); }
                if (ImGui::MenuItem("重做", "Ctrl+Y")) { document.Redo(); }
                
                
                ImGui::Separator();
                ImGui::MenuItem("剪切", "Ctrl+X",false,false);
                ImGui::MenuItem("复制", "Ctrl+C", false, false); 
                ImGui::MenuItem("粘贴", "Ctrl+V", false, false); 
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("视图"))
            {
                ImGui::MenuItem("场景", "默认", false, false);
                ImGui::MenuItem("特性", "Ctrl+P", false, false);
                ImGui::Separator();
                ImGui::MenuItem("工具栏", "Ctrl+T", false, false);
                ImGui::MenuItem("资源管理器", "Ctrl+E", false, false);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("工具"))
            { 
                if (ImGui::MenuItem("网格", "G",   false, false))
                {
                    // bool cur = document.GetEditor().IsGridShown();
                    // document.GetEditor().SetShowGrid(!cur);
                }
                if (ImGui::MenuItem("轴线", "A",  false, false))
                {
                    // bool cur = document.GetEditor().IsAxisShown();
                    // document.GetEditor().SetShowAxis(!cur);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Guide", "Ctrl+I", false, false)) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("扩展"))
            {
                if (ImGui::MenuItem("Demo", "", false, false)) {}
                //ImGui::Separator();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("帮助"))
            {
                if (ImGui::MenuItem("在线", "", false, false)) {} 
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
}