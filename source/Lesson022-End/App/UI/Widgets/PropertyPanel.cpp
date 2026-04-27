#include "PropertyPanel.h"
#include <imgui.h> 
#include "App/Document/Document.h"
#include "Core/Entity/LineEntity.hpp"
#include "App/Command/UpdateLineEntityCommand.h"
#include "PropertySubPanelLine.h"
namespace MiniCAD
{
    PropertyPanel::PropertyPanel()
    {
        m_id = "property_panel";

        m_subPanels[typeid(LineEntity)] = std::make_unique<PropertySubPanelLine>();  
    }

    const char* PropertyPanel::GetName() const { return "特性"; }
     
    void PropertyPanel::OnRender(Document& document)
    { 
        //  设置窗口背景颜色
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
        if (!ImGui::Begin(GetName()))
        {
            ImGui::End();
            ImGui::PopStyleColor();
            return;
        }

        auto* object = document.GetEditor().GetPrimarySelectedObject();
        if (!object)
        {
            ImGui::Text("未选择对象");
            ImGui::End();
            ImGui::PopStyleColor();
            return;
        }

        // 按实际运行时类型分发 
        auto it = m_subPanels.find(typeid(*object));

        if (it != m_subPanels.end())
            it->second->OnRender(object, document);
        else
            ImGui::Text("未知对象类型");

        ImGui::End();
        ImGui::PopStyleColor(); //  End 后 Pop
    }
}