#pragma once
#include "PropertySubPanelLine.h" 
#include "Core/Object/Object.hpp"
#include "Core/Entity/LineEntity.hpp"
#include "App/Document/Document.h"
#include "App/Command/UpdateLineEntityCommand.h"
#include "App/Command/BatchCommand.h"
#include <imgui.h> 
#include <memory>
namespace MiniCAD
{
	void PropertySubPanelLine::OnRender(Object* entity, Document& document)
	{  
        auto* object = document.GetEditor().GetPrimarySelectedObject();

        auto  objectids = document.GetEditor().GetSelection(); 
        
        if (!object)
        {
            ImGui::Text("未选择对象"); 
            return;
        } 

        if (object->IsKindOf<LineEntity>())
        {
            // 标题直线
            ImGui::Indent(5.0);
            ImGui::SetWindowFontScale(1.2f);
            ImGui::Text("直线");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Unindent(5.0);

            auto* lineEntity = static_cast<LineEntity*>(object);

            Line before           = lineEntity->GetLine();
            Line temp             = before; 
            EntityAttr attrBefore = lineEntity->GetAttr(); // 拷贝，保存原始状态
            EntityAttr attrTemp   = attrBefore;              // 拷贝，用于UI修改

            bool anyDeactivated = false;
             
              
            // --- 常规 ---
            ImGui::Separator();
            ImGui::Indent(10.0f);
            ImGui::SetWindowFontScale(1.2f);
            ImGui::Text("常规");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Unindent(10.0f); 
            ImGui::Separator();
            ImGui::Indent(10.0f);

            // 1. 颜色（XMFLOAT4，含Alpha）    
            ImGui::Text("颜色");
            ImGui::SameLine(m_labelWidth);
            ImGui::PushItemWidth(m_inputWidth);

            // 预设颜色表（你可以自己扩展）
            int colorIndex = 0;

            auto& cur = attrTemp.Color;
            for (int i = 0; i < IM_ARRAYSIZE(colors); i++)
            {
                const auto& c = colors[i].value;

                if (fabs(c.x - cur.x) < 0.01f &&
                    fabs(c.y - cur.y) < 0.01f &&
                    fabs(c.z - cur.z) < 0.01f &&
                    fabs(c.w - cur.w) < 0.01f)
                {
                    colorIndex = i;
                    break;
                }
            }
            if (ImGui::BeginCombo("##选择颜色", colors[colorIndex].name))
            {
                for (int i = 0; i < IM_ARRAYSIZE(colors); i++)
                {
                    ImGui::PushID(i); // ⭐关键：让每一行都有独立ID

                    bool selected = (i == colorIndex);

                    ImGui::ColorButton(
                        "##col",
                        ImVec4(colors[i].value.x, colors[i].value.y, colors[i].value.z, colors[i].value.w),
                        ImGuiColorEditFlags_NoTooltip,
                        ImVec2(18, 18)
                    );

                    ImGui::SameLine();

                    if (ImGui::Selectable(colors[i].name, selected))
                    {
                        colorIndex = i;
                        attrTemp.Color = colors[i].value;
                        anyDeactivated = true;
                    }

                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();

            // 2. 线型
            ImGui::Text("线型");
            ImGui::SameLine(m_labelWidth);
            ImGui::PushItemWidth(m_inputWidth);
            const char* lineTypes[] = { "实线"/*, "虚线", "点线", "点划线"*/ };
            int lineTypeIndex = static_cast<int>(attrTemp.LineType);
            if (ImGui::Combo("##线型", &lineTypeIndex, lineTypes, IM_ARRAYSIZE(lineTypes)))
            {
                attrTemp.LineType = static_cast<MiniCAD::LineType>(lineTypeIndex);
                anyDeactivated = true;
            }
            ImGui::PopItemWidth();

            // 3. 线宽
            ImGui::Text("线宽");
            ImGui::SameLine(m_labelWidth);
            ImGui::PushItemWidth(m_inputWidth);
            ImGui::InputFloat("##线宽", &attrTemp.LineWidth, 0.0f, 0.0f, "%.2f");
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::PopItemWidth();

            // 4. 透明度（来自 Color.w）
            ImGui::Text("透明度");
            ImGui::SameLine(m_labelWidth);
            ImGui::PushItemWidth(m_inputWidth);
            ImGui::SliderFloat("##透明度", &attrTemp.Color.w, 0.0f, 1.0f, "%.2f");
            // 同样改成 IsItemDeactivatedAfterEdit
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::PopItemWidth();

            // 5. 可见性
            ImGui::Text("可见");
            ImGui::SameLine(m_labelWidth);
            if (ImGui::Checkbox("##可见", &attrTemp.Visible))
            {
                anyDeactivated = true;
            }
              
            ImGui::Unindent(10.0f);
            ImGui::Separator();
            // --- 几何图形-----------  
            auto FloatFieldLeft = [&](const char* label, float* v, bool readOnly = false) -> bool
            {
                ImGui::Text("%s", label);
                ImGui::SameLine(m_labelWidth);
                ImGui::PushItemWidth(m_inputWidth);
                std::string id = std::string("##") + label;
                ImGuiInputTextFlags flags = 0;

                if (readOnly)
                {
                    flags = ImGuiInputTextFlags_ReadOnly;
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                    // ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                    // ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                    // ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                }

                bool ret = ImGui::InputFloat(id.c_str(), v, 0.0f, 0.0f, "%.4f", flags);

                if (readOnly)
                { 
                    ImGui::PopStyleColor(1);
                }

                ImGui::PopItemWidth();
                return ret;
            };

            ImGui::Indent(10.0f);
            ImGui::SetWindowFontScale(1.2f);
            ImGui::Text("几何图形");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Unindent(10.0f);

            ImGui::Separator();
            ImGui::Indent(10.0f);

            // 起点终点（可编辑）
            FloatFieldLeft("起点 X 坐标", &temp.Start.x);
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();
            FloatFieldLeft("起点 Y 坐标", &temp.Start.y);
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();
            FloatFieldLeft("起点 Z 坐标", &temp.Start.z);
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();

            FloatFieldLeft("终点 X 坐标", &temp.End.x);
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();
            FloatFieldLeft("终点 Y 坐标", &temp.End.y);
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();
            FloatFieldLeft("终点 Z 坐标", &temp.End.z);
            anyDeactivated |= ImGui::IsItemDeactivatedAfterEdit();

            ImGui::Separator();

            // 实时计算派生属性
            float deltaX = temp.End.x - temp.Start.x;
            float deltaY = temp.End.y - temp.Start.y;
            float deltaZ = temp.End.z - temp.Start.z;
            float length = std::sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
              
            constexpr float PI = 3.14159265358979323846f;
            float angle = std::atan2(deltaY, deltaX) * (180.0f / PI); 

            // 只读显示
            FloatFieldLeft("增量 X", &deltaX, true);
            FloatFieldLeft("增量 Y", &deltaY, true);
            FloatFieldLeft("增量 Z", &deltaZ, true);
            FloatFieldLeft("长度",   &length, true);
            FloatFieldLeft("角度",   &angle,  true);


            if (anyDeactivated)
            {
                auto objectids = document.GetEditor().GetSelection();

                if (objectids.size() == 1)
                {
                    // 单个修改
                    document.GetEditor().ExecuteCommand(
                        std::make_unique<UpdateLineEntityCommand>(
                            lineEntity->GetID(),
                            LineEntityState{ before, attrBefore },
                            LineEntityState{ temp, attrTemp }
                        )
                    );
                }
                else
                {
                    // 批量修改
                    std::vector<std::unique_ptr<ICommand>> cmds;

                    for (auto id : objectids)
                    {
                        auto* obj = document.GetScene().GetEntity(id);
                        if (!obj || !obj->IsKindOf<LineEntity>()) continue;

                        auto* le = static_cast<LineEntity*>(obj);

                        LineEntityState bs, as;
                        bs.line = le->GetLine();
                        bs.attr = le->GetAttr();
                        as.line = bs.line;         // 几何坐标各自保留
                        as.attr = attrTemp;        // 属性统一应用主选对象的值

                        cmds.push_back(std::make_unique<UpdateLineEntityCommand>(id, bs, as));
                    }

                    document.GetEditor().ExecuteCommand(std::make_unique<BatchCommand>(std::move(cmds)));
                }
            }
        }
         
	}
}
