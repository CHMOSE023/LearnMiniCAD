#include "ToolBar.h"
#include <imgui.h>
#include "App/Document/Document.h"
#include <imgui_internal.h>
#include "Core/GeomKernel/Line.hpp"
#include "Core/Entity/LineEntity.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <memory>

namespace MiniCAD
{
    const char* ToolBar::GetName() const { return "工具栏"; }
     
    ToolBar::ToolBar()
    {
        m_id = "tool_bar";
    }

    void ToolBar::OnRender(Document& document)
    {
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


        auto& editor = document.GetEditor();

        if (ImGui::Button("直线"))
            editor.StartLineTool();

        ImGui::SameLine();

        if (ImGui::Button("点"))
        {
            editor.StartPointTool();
        }

        ImGui::SameLine();

        if (ImGui::Button("一键轴网"))
            m_showAxisWindow = true;

        ImGui::End();

        if (m_showAxisWindow)
            RenderAxisWindow(document);
    }

    void ToolBar::RenderAxisWindow(Document& document)
    {
        // ── 辅助 lambda ──────────────────────────────────────────────────

        auto syncPos = [&]()
            {
                for (int dir = 0; dir <= 1; ++dir)
                {
                    bool isV = (dir == 1);
                    float mn = FLT_MAX, mx = -FLT_MAX;
                    for (auto& a : m_axes)
                        if (a.vertical == isV) { mn = std::min(mn, a.realPos); mx = std::max(mx, a.realPos); }
                    if (mn == FLT_MAX) continue;
                    float range = mx - mn;
                    for (auto& a : m_axes)
                    {
                        if (a.vertical != isV) continue;
                        a.pos = (range > 1e-6f) ? 0.05f + (a.realPos - mn) / range * 0.90f : 0.5f;
                    }
                }
            };

        auto syncRealFromPos = [&](int idx)
            {
                bool isV = m_axes[idx].vertical;
                float mn = FLT_MAX, mx = -FLT_MAX, pmin = FLT_MAX, pmax = -FLT_MAX;
                for (auto& a : m_axes)
                    if (a.vertical == isV)
                    {
                        mn = std::min(mn, a.realPos); mx = std::max(mx, a.realPos);
                        pmin = std::min(pmin, a.pos);  pmax = std::max(pmax, a.pos);
                    }
                float pr = pmax - pmin, rr = mx - mn;
                if (pr > 1e-6f && rr > 1e-6f)
                    m_axes[idx].realPos = mn + (m_axes[idx].pos - pmin) / pr * rr;
            };

        auto calcRange = [&](float& vMin, float& vMax, float& hMin, float& hMax)
            {
                vMin = FLT_MAX; vMax = -FLT_MAX;
                hMin = FLT_MAX; hMax = -FLT_MAX;
                for (auto& a : m_axes)
                {
                    if (a.vertical) { vMin = std::min(vMin, a.realPos); vMax = std::max(vMax, a.realPos); }
                    else { hMin = std::min(hMin, a.realPos); hMax = std::max(hMax, a.realPos); }
                }
                if (vMin == FLT_MAX) { vMin = 0.0f; vMax = 10.0f; }
                if (hMin == FLT_MAX) { hMin = 0.0f; hMax = 10.0f; }
            };

        // ── 窗口 ─────────────────────────────────────────────────────────
        ImGui::SetNextWindowSize(ImVec2(960, 520), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("轴网", &m_showAxisWindow))
        {
            ImGui::End();
            return;
        }

        // ── 顶部工具条 ────────────────────────────────────────────────────
        ImGui::Text("方向："); ImGui::SameLine();
        ImGui::RadioButton("纵向", &m_addVertical, 1); ImGui::SameLine();
        ImGui::RadioButton("横向", &m_addVertical, 0); ImGui::SameLine();

        if (ImGui::Button("添加轴"))
        {
            Axis a;
            a.vertical = (m_addVertical != 0);
            a.visible = true;
            float maxR = 0.0f;
            for (auto& x : m_axes) if (x.vertical == a.vertical) maxR = std::max(maxR, x.realPos);
            a.realPos = m_axes.empty() ? 0.0f : maxR + 3.0f;
            a.pos = 0.5f;
            std::ostringstream ss; ss << "轴" << m_axisCounter++;
            a.name = ss.str();
            m_axes.push_back(std::move(a));
            syncPos();
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::InputFloat("出头(m)", &m_axisExtend, 0.5f, 1.0f, "%.1f");
        m_axisExtend = std::max(0.0f, m_axisExtend);

        ImGui::Text("原点："); ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);
        ImGui::InputFloat("X##ox", &m_gridOrigin.x, 1.0f, 10.0f, "%.2f");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);
        ImGui::InputFloat("Y##oy", &m_gridOrigin.y, 1.0f, 10.0f, "%.2f");

        ImGui::Columns(2, "axis_cols");

        // ── 左侧画布（内容不变，照搬原逻辑）────────────────────────────
        ImGui::BeginChild("AxisCanvas", ImVec2(0, 400), true);
        {
            // ... 原有画布绘制代码，把 axes/dragging 换成 m_axes/m_dragging ...
            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 CP = ImGui::GetCursorScreenPos();
                ImVec2 CS = ImGui::GetContentRegionAvail();
                if (CS.x < 20) CS.x = 460;
                if (CS.y < 20) CS.y = 380;

                // 背景
                dl->AddRectFilled(CP, ImVec2(CP.x + CS.x, CP.y + CS.y), IM_COL32(12, 12, 12, 255));

                // ── 画布布局 ──────────────────────────────────────────────
                // 留出 margin 给轴号圆泡
                const float MARGIN = 32.0f;   // 圆泡区域（像素）
                const float BUBBLE_R = 10.0f;  // 圆泡半径

                float gL = CP.x + MARGIN;     // grid left
                float gR = CP.x + CS.x - MARGIN; // grid right
                float gT = CP.y + MARGIN;     // grid top
                float gB = CP.y + CS.y - MARGIN; // grid bottom
                float gW = gR - gL;
                float gH = gB - gT;

                // 网格边框（淡色参考框）
                dl->AddRect(ImVec2(gL, gT), ImVec2(gR, gB), IM_COL32(50, 50, 50, 180), 0, 0, 1.0f);

                ImVec2 mouse = ImGui::GetIO().MousePos;
                bool   hov = ImGui::IsWindowHovered();

                // ── 辅助：按 pos 排序的同向轴索引 ──────────────────────
                auto sorted = [&](bool vert) -> std::vector<int>
                    {
                        std::vector<int> idx;
                        for (int i = 0; i < (int)m_axes.size(); ++i)
                            if (m_axes[i].vertical == vert && m_axes[i].visible)
                                idx.push_back(i);
                        std::sort(idx.begin(), idx.end(),
                            [&](int a, int b) { return m_axes[a].pos < m_axes[b].pos; });
                        return idx;
                    };

                ImU32 lineCol = IM_COL32(200, 200, 50, 255);
                ImU32 extCol = IM_COL32(200, 200, 50, 120);
                ImU32 bubbleBg = IM_COL32(30, 30, 30, 230);
                ImU32 bubbleFg = IM_COL32(220, 200, 60, 255);
                ImU32 dimCol = IM_COL32(100, 220, 255, 220);
                ImU32 tickCol = IM_COL32(100, 220, 255, 140);

                auto drawBubble = [&](float cx, float cy, const std::string& name)
                    {
                        dl->AddCircleFilled(ImVec2(cx, cy), BUBBLE_R, bubbleBg);
                        dl->AddCircle(ImVec2(cx, cy), BUBBLE_R, bubbleFg, 0, 1.5f);
                        ImVec2 tsz = ImGui::CalcTextSize(name.c_str());
                        dl->AddText(ImVec2(cx - tsz.x * 0.5f, cy - tsz.y * 0.5f), bubbleFg, name.c_str());
                    };

                for (size_t i = 0; i < m_axes.size(); ++i)
                {
                    if (!m_axes[i].visible) continue;

                    if (m_axes[i].vertical)
                    {
                        float x = gL + m_axes[i].pos * gW;

                        // 主体：网格范围内（实线）
                        dl->AddLine(ImVec2(x, gT), ImVec2(x, gB), lineCol, 1.5f);
                        // 出头：超出网格到 margin 区（淡线）
                        dl->AddLine(ImVec2(x, CP.y + MARGIN * 0.15f), ImVec2(x, gT), extCol, 1.5f);
                        dl->AddLine(ImVec2(x, gB), ImVec2(x, CP.y + CS.y - MARGIN * 0.15f), extCol, 1.5f);
                        // 圆泡
                        drawBubble(x, CP.y + BUBBLE_R + 2.0f, m_axes[i].name);
                        drawBubble(x, CP.y + CS.y - BUBBLE_R - 2.0f, m_axes[i].name);

                        // 拖拽命中检测
                        if (hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                            if (std::abs(mouse.x - x) < 6.0f && mouse.y > gT && mouse.y < gB)
                                m_dragging = (int)i;
                    }
                    else
                    {
                        float y = gT + m_axes[i].pos * gH;

                        dl->AddLine(ImVec2(gL, y), ImVec2(gR, y), lineCol, 1.5f);
                        dl->AddLine(ImVec2(CP.x + MARGIN * 0.15f, y), ImVec2(gL, y), extCol, 1.5f);
                        dl->AddLine(ImVec2(gR, y), ImVec2(CP.x + CS.x - MARGIN * 0.15f, y), extCol, 1.5f);
                        drawBubble(CP.x + BUBBLE_R + 2.0f, y, m_axes[i].name);
                        drawBubble(CP.x + CS.x - BUBBLE_R - 2.0f, y, m_axes[i].name);

                        if (hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                            if (std::abs(mouse.y - y) < 6.0f && mouse.x > gL && mouse.x < gR)
                                m_dragging = (int)i;
                    }
                }

                // ── 距离标注（相邻同向轴之间）──────────────────────────
                // 纵向：尺寸线画在网格顶部内侧
                {
                    auto idx = sorted(true);
                    for (int k = 1; k < (int)idx.size(); ++k)
                    {
                        int  a = idx[k - 1], b = idx[k];
                        float dist = std::abs(m_axes[b].realPos - m_axes[a].realPos);
                        float xa = gL + m_axes[a].pos * gW;
                        float xb = gL + m_axes[b].pos * gW;
                        float xm = (xa + xb) * 0.5f;
                        float y0 = gT + 6.0f;

                        dl->AddLine(ImVec2(xa, y0), ImVec2(xa, y0 + 8.0f), tickCol, 1.0f);
                        dl->AddLine(ImVec2(xb, y0), ImVec2(xb, y0 + 8.0f), tickCol, 1.0f);
                        dl->AddLine(ImVec2(xa, y0 + 4.0f), ImVec2(xb, y0 + 4.0f), tickCol, 1.0f);

                        char buf[32]; snprintf(buf, sizeof(buf), "%.2fm", dist);
                        ImVec2 tsz = ImGui::CalcTextSize(buf);
                        dl->AddRectFilled(
                            ImVec2(xm - tsz.x * 0.5f - 2, y0 + 10.0f),
                            ImVec2(xm + tsz.x * 0.5f + 2, y0 + 10.0f + tsz.y), IM_COL32(12, 12, 12, 200));
                        dl->AddText(ImVec2(xm - tsz.x * 0.5f, y0 + 10.0f), dimCol, buf);
                    }
                }

                // 横向：尺寸线画在网格左侧内侧
                {
                    auto idx = sorted(false);
                    for (int k = 1; k < (int)idx.size(); ++k)
                    {
                        int  a = idx[k - 1], b = idx[k];
                        float dist = std::abs(m_axes[b].realPos - m_axes[a].realPos);
                        float ya = gT + m_axes[a].pos * gH;
                        float yb = gT + m_axes[b].pos * gH;
                        float ym = (ya + yb) * 0.5f;
                        float x0 = gL + 6.0f;

                        dl->AddLine(ImVec2(x0, ya), ImVec2(x0 + 8.0f, ya), tickCol, 1.0f);
                        dl->AddLine(ImVec2(x0, yb), ImVec2(x0 + 8.0f, yb), tickCol, 1.0f);
                        dl->AddLine(ImVec2(x0 + 4.0f, ya), ImVec2(x0 + 4.0f, yb), tickCol, 1.0f);

                        char buf[32]; snprintf(buf, sizeof(buf), "%.2fm", dist);
                        ImVec2 tsz = ImGui::CalcTextSize(buf);
                        dl->AddRectFilled(
                            ImVec2(x0 + 12.0f, ym - tsz.y * 0.5f - 1),
                            ImVec2(x0 + 12.0f + tsz.x + 4, ym + tsz.y * 0.5f + 1), IM_COL32(12, 12, 12, 200));
                        dl->AddText(ImVec2(x0 + 14.0f, ym - tsz.y * 0.5f), dimCol, buf);
                    }
                }

                // ── 拖拽 ──────────────────────────────────────────────
                if (m_dragging >= 0 && m_dragging < (int)m_axes.size() &&
                    ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    if (m_axes[m_dragging].vertical)
                        m_axes[m_dragging].pos = std::clamp((mouse.x - gL) / gW, 0.0f, 1.0f);
                    else
                        m_axes[m_dragging].pos = std::clamp((mouse.y - gT) / gH, 0.0f, 1.0f);
                    syncRealFromPos(m_dragging);
                }
                if (m_dragging >= 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    m_dragging = -1;

                ImGui::EndChild();
            }  
        } 

        ImGui::NextColumn();

        // ── 右侧轴表 ──────────────────────────────────────────────────────
        ImGui::BeginChild("AxisTable", ImVec2(0, 400), false);
        {
            // ... 原有轴表代码，把 axes 换成 m_axes ...

            {
                bool needSync = false;

                if (ImGui::BeginTable("atc", 5,
                    ImGuiTableFlags_Resizable |
                    ImGuiTableFlags_BordersInnerV |
                    ImGuiTableFlags_ScrollY,
                    ImVec2(0, 0)))
                {
                    ImGui::TableSetupColumn("名称", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("方向", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                    ImGui::TableSetupColumn("位置(m)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("显示", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                    ImGui::TableSetupColumn("操作", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (int i = 0; i < (int)m_axes.size(); ++i)
                    {
                        ImGui::TableNextRow();
                        ImGui::PushID(i);

                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(m_axes[i].name.c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(m_axes[i].vertical ? "纵" : "横");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::PushItemWidth(-1);
                        if (ImGui::InputFloat("##p", &m_axes[i].realPos, 0.5f, 1.0f, "%.2f"))
                            needSync = true;
                        ImGui::PopItemWidth();

                        ImGui::TableSetColumnIndex(3);
                        ImGui::Checkbox("##v", &m_axes[i].visible);

                        ImGui::TableSetColumnIndex(4);
                        if (ImGui::SmallButton("中")) { m_axes[i].realPos = 0.0f; needSync = true; }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("X"))
                        {
                            m_axes.erase(m_axes.begin() + i);
                            ImGui::PopID();
                            needSync = true;
                            break;
                        }

                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }

                // 间距一览
                if (!m_axes.empty())
                {
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "间距一览");

                    std::vector<int> vIdx, hIdx;
                    for (int i = 0; i < (int)m_axes.size(); ++i)
                        (m_axes[i].vertical ? vIdx : hIdx).push_back(i);
                    auto cmp = [&](int a, int b) { return m_axes[a].realPos < m_axes[b].realPos; };
                    std::sort(vIdx.begin(), vIdx.end(), cmp);
                    std::sort(hIdx.begin(), hIdx.end(), cmp);

                    if (vIdx.size() >= 2) {
                        ImGui::TextDisabled("纵轴：");
                        for (int k = 1; k < (int)vIdx.size(); ++k)
                            ImGui::Text("  %s → %s : %.2f m",
                                m_axes[vIdx[k - 1]].name.c_str(), m_axes[vIdx[k]].name.c_str(),
                                m_axes[vIdx[k]].realPos - m_axes[vIdx[k - 1]].realPos);
                    }
                    if (hIdx.size() >= 2) {
                        ImGui::TextDisabled("横轴：");
                        for (int k = 1; k < (int)hIdx.size(); ++k)
                            ImGui::Text("  %s → %s : %.2f m",
                                m_axes[hIdx[k - 1]].name.c_str(), m_axes[hIdx[k]].name.c_str(),
                                m_axes[hIdx[k]].realPos - m_axes[hIdx[k - 1]].realPos);
                    }
                }

                if (needSync) syncPos();
            }

        }
        ImGui::EndChild();

        ImGui::Columns(1);
        ImGui::Separator();

        // ── 确定 / 取消 ───────────────────────────────────────────────────
        if (ImGui::Button("确定"))
        {
            float vMin, vMax, hMin, hMax;
            calcRange(vMin, vMax, hMin, hMax);

            auto& scene = document.GetScene();
            for (auto& a : m_axes)
            {
                if (!a.visible) continue;
                XMFLOAT3 start, end;
                if (a.vertical)
                {
                    float wx = m_gridOrigin.x + (a.realPos - vMin);
                    float wyMin = m_gridOrigin.y - m_axisExtend;
                    float wyMax = m_gridOrigin.y + (hMax - hMin) + m_axisExtend;
                    start = { wx, wyMin, 0.0f };
                    end = { wx, wyMax, 0.0f };
                }
                else
                {
                    float wy = m_gridOrigin.y + (a.realPos - hMin);
                    float wxMin = m_gridOrigin.x - m_axisExtend;
                    float wxMax = m_gridOrigin.x + (vMax - vMin) + m_axisExtend;
                    start = { wxMin, wy, 0.0f };
                    end = { wxMax, wy, 0.0f };
                }
                if (!MiniCAD::Line(start, end, true).IsValid()) continue;
                scene.AddEntity(std::make_unique<LineEntity>(scene.NextObjectID(), start, end));
            }
            m_showAxisWindow = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("取消")) m_showAxisWindow = false;

        ImGui::End();
    }
     
}