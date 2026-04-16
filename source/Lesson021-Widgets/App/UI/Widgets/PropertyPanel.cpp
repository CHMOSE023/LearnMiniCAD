#include "PropertyPanel.h"
#include <imgui.h> 
#include "App/Document/Document.h"
#include "Core/Entity/LineEntity.hpp"
namespace MiniCAD
{
    const char* PropertyPanel::GetName() const
    {
        return "特性";
    }

    void PropertyPanel::OnRender(Document& document)
    {
        if (!ImGui::Begin(GetName()))
        {
            ImGui::End();
            return;
        }

        auto* object = document.GetEditor().GetPrimarySelectedObject();

        if (!object)
        {
            ImGui::Text("未选择对象");
            ImGui::End();
            return;
        }


        if (object->IsKindOf<LineEntity>())
        {
            auto* lineEntity = static_cast<LineEntity*>(object);
            auto line = lineEntity->GetLine();  

            ImGui::Text("直线");
            ImGui::Separator();

            ImGui::DragFloat3("起点", &line.Start.x, 0.1f);
            ImGui::DragFloat3("终点", &line.End.x, 0.1f);
        }

        ImGui::End();
    }
}