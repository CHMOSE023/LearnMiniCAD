#include "App/UI/Widgets/LayerManagerWidget.h" 
#include <imgui.h> 
#include "App/Document/Document.h"
#include "App/Scene/LayerManager.h"
#include "App/Scene/Layer.h" 

#include <algorithm>
#include <cstring>

namespace MiniCAD
{
    LayerManagerWidget::LayerManagerWidget() { m_id = "layer_manager_widget"; }

    // ─────────────────────────────────────────────────────────────────────────────
    //  OnRender
    // ─────────────────────────────────────────────────────────────────────────────
    void LayerManagerWidget::OnRender(Document& document)
    { 
        LayerManager& mgr = document.GetLayerManager();
          
        if (!m_visible)
            return;

        bool open = m_visible;

        if (!ImGui::Begin(GetName(), &open))
        {
            ImGui::End();
            return;
        }

        if (!open)  // 如果用户点了右上角关闭
            m_visible = false; 

        if (m_visible)
        {
            RenderToolbar(mgr);
            ImGui::Separator();
            RenderLayerList(mgr);
        }

        RenderAddDialog(mgr); 

        ImGui::End();
    }

    // ─────────────────────────────────────────────────────────────────────────────
    //  工具栏
    // ─────────────────────────────────────────────────────────────────────────────
    void LayerManagerWidget::RenderToolbar(LayerManager& mgr)
    {
        if (ImGui::Button("+ 新建图层"))
        {
            m_newLayerName[0] = '\0';
            ImGui::OpenPopup("Add Layer##dialog");
        }

        ImGui::SameLine();

        const LayerID activeID = mgr.GetActiveLayerID();
        const bool    canDelete = (activeID != Layer::DefaultLayerID);

        if (!canDelete) ImGui::BeginDisabled();

        if (ImGui::Button("- 删除图层"))
            mgr.RemoveLayer(activeID);

        if (!canDelete) ImGui::EndDisabled();

        if (!canDelete && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Cannot remove the default layer.");
    }

    // ─────────────────────────────────────────────────────────────────────────────
    //  图层列表
    // ─────────────────────────────────────────────────────────────────────────────
    void LayerManagerWidget::RenderLayerList(LayerManager& mgr)
    {
        constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg            |
                                               ImGuiTableFlags_BordersInnerV    |
                                               ImGuiTableFlags_ScrollY          |
                                               ImGuiTableFlags_Resizable        |
                                               ImGuiTableFlags_SizingStretchProp;
         
        const ImVec2 outerSize{ 0.f, ImGui::GetContentRegionAvail().y };

        if (!ImGui::BeginTable("LayerTable", 4, tableFlags, outerSize))
            return;

        // ── 列定义 ───────────────────────────────────────────────────────────────
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableSetupColumn("颜色",
            ImGuiTableColumnFlags_WidthFixed, 40.f);

        ImGui::TableSetupColumn("名称",
            ImGuiTableColumnFlags_WidthStretch, 1.0f);

        ImGui::TableSetupColumn("可见",
            ImGuiTableColumnFlags_WidthFixed, 60.f);

        ImGui::TableSetupColumn("锁定",
            ImGuiTableColumnFlags_WidthFixed, 60.f);

        ImGui::TableHeadersRow();

        const LayerID activeID = mgr.GetActiveLayerID();
        auto ids = mgr.GetAllLayerIDs();

        std::stable_sort(ids.begin(), ids.end(),
            [](LayerID a, LayerID b)
            {
                if (a == Layer::DefaultLayerID) return true;
                if (b == Layer::DefaultLayerID) return false;
                return a < b;
            });

        static LayerID s_renamingID = static_cast<LayerID>(0xFFFFFFFFu);
        static char    s_renameBuf[64]{};

        for (LayerID id : ids)
        {
            Layer* layer = mgr.GetLayer(id);
            if (!layer) continue;

            ImGui::PushID(static_cast<int>(id));
            ImGui::TableNextRow();

            // ── Col 0 : Color ────────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(0);
            {
                const auto& col = layer->GetColor();
                ImVec4 imcol{ col.x, col.y, col.z, col.w };

                ImGui::ColorButton("##cb", imcol,
                    ImGuiColorEditFlags_NoTooltip |
                    ImGuiColorEditFlags_NoBorder,
                    ImVec2{ 16.f, 16.f });

                if (ImGui::IsItemClicked())
                {
                    m_colorPickerLayerID = id;
                    m_colorPickerShouldOpen = true; // 只标记，EndTable 后统一 OpenPopup
                }
            }

            // ── Col 1 : Name ─────────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(1);
            {
                const bool isActive = (id == activeID);

                if (isActive)
                {
                    ImGui::TableSetBgColor(
                        ImGuiTableBgTarget_RowBg0,
                        ImGui::GetColorU32(ImGuiCol_Header));
                }

                char selLabel[32];
                std::snprintf(selLabel, sizeof(selLabel), "##sel%u", id);

                if (ImGui::Selectable(
                    selLabel,
                    isActive,
                    ImGuiSelectableFlags_SpanAllColumns |
                    ImGuiSelectableFlags_AllowOverlap,
                    ImVec2{ 0.f, 0.f }))
                {
                    mgr.SetActiveLayerID(id);
                }

                ImGui::SameLine();

                // ── Rename 状态 ──────────────────────────────────────────────────
                if (s_renamingID == id)
                {
                    ImGui::SetNextItemWidth(-1.f);

                    if (ImGui::IsWindowAppearing())
                        ImGui::SetKeyboardFocusHere();

                    bool confirmed = ImGui::InputText(
                        "##rename",
                        s_renameBuf,
                        sizeof(s_renameBuf),
                        ImGuiInputTextFlags_EnterReturnsTrue |
                        ImGuiInputTextFlags_AutoSelectAll);

                    if (confirmed)
                    {
                        if (s_renameBuf[0] != '\0')
                            layer->SetName(s_renameBuf);

                        s_renamingID = static_cast<LayerID>(0xFFFFFFFFu);
                    }
                    else if (ImGui::IsItemDeactivated() ||
                        ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        s_renamingID = static_cast<LayerID>(0xFFFFFFFFu);
                    }
                }
                else
                {
                    ImGui::TextUnformatted(layer->GetName().c_str());

                    if (ImGui::IsItemHovered() &&
                        ImGui::IsMouseDoubleClicked(0))
                    {
                        s_renamingID = id;
                        std::strncpy(s_renameBuf,
                            layer->GetName().c_str(),
                            sizeof(s_renameBuf));
                        s_renameBuf[sizeof(s_renameBuf) - 1] = '\0';
                    }
                }
            }

            // ── Col 2 : Visible ──────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(2);
            {
                bool visible = layer->IsVisible();
                if (ImGui::Checkbox("##vis", &visible))
                    layer->SetVisible(visible);
            }

            // ── Col 3 : Locked ───────────────────────────────────────────────────
            ImGui::TableSetColumnIndex(3);
            {
                bool locked = layer->IsLocked();
                if (ImGui::Checkbox("##lck", &locked))
                    layer->SetLocked(locked);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();

        // ── Color Picker Popup ─────────────────────────────────────────────────── 
        if (m_colorPickerShouldOpen)
        {
            ImGui::OpenPopup("LayerColorPicker");
            m_colorPickerShouldOpen = false; // 立即清除，下帧不再触发
        }

        if (ImGui::BeginPopup("LayerColorPicker"))
        {
            Layer* target = mgr.GetLayer(m_colorPickerLayerID);

            if (target)
            {
                float rgba[4] = {
                    target->GetColor().x,
                    target->GetColor().y,
                    target->GetColor().z,
                    target->GetColor().w
                };

                ImGui::Text("Layer: %s", target->GetName().c_str());
                ImGui::Separator();

                if (ImGui::ColorPicker4("##picker", rgba))
                    target->SetColor({ rgba[0], rgba[1], rgba[2], rgba[3] });
            }

            if (ImGui::Button("确认关闭"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
        else
        {
            // 弹窗被任意方式关闭（点空白 / 确认关闭）后清理 ID
            m_colorPickerLayerID = kInvalidID;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────────
    //  新建图层对话框
    // ─────────────────────────────────────────────────────────────────────────────
    void LayerManagerWidget::RenderAddDialog(LayerManager& mgr)
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, { 0.5f, 0.5f });

        if (!ImGui::BeginPopupModal("Add Layer##dialog", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
            return;

        ImGui::Text("图层名称:");
        ImGui::SetNextItemWidth(220.f);
         
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();

        bool enterPressed = ImGui::InputText("##newname",
            m_newLayerName, sizeof(m_newLayerName),
            ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::Spacing();

        bool ok = ImGui::Button("添加", { 100.f, 0.f });
        ImGui::SameLine();
        bool cancel = ImGui::Button("取消", { 100.f, 0.f });

        if ((ok || enterPressed) && m_newLayerName[0] != '\0')
        {
            LayerID newID = mgr.AddLayer(m_newLayerName);
            mgr.SetActiveLayerID(newID);
            ImGui::CloseCurrentPopup();
        }
        else if (cancel)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

} 
