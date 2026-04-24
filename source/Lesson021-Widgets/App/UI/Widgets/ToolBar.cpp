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

    void ToolBar::OnRender(Document& document)
    {
        ImGui::Begin(GetName());
        {
            auto& editor = document.GetEditor();
            ImGui::BeginGroup();
            if (ImGui::Button("直线", ImVec2(60, 32))) editor.StartLineTool();
            ImGui::SameLine();
            if (ImGui::Button("点", ImVec2(60, 32))) {}
            ImGui::SameLine();
            static bool showAxisWindow = false;
            if (ImGui::Button("一键轴网", ImVec2(60, 32))) showAxisWindow = true;
            ImGui::EndGroup();

            if (!showAxisWindow) { ImGui::End(); return; }

            ImGui::SetNextWindowSize(ImVec2(960, 520), ImGuiCond_FirstUseEver);
            ImGui::Begin("轴网", &showAxisWindow);
            {
                struct Axis { float pos; float realPos; bool vertical; bool visible; std::string name; };
                static std::vector<Axis> axes;
                static int      axisCounter = 1;
                static int      addVertical = 1;
                static int      dragging = -1;
                static float    axisExtend = 1.5f;   // 轴线出头（米）
                static XMFLOAT2 gridOrigin = { 0.0f, 0.0f }; // 轴网左下角交叉点世界坐标

                // ── realPos → 归一化 pos [0,1] ──────────────────────────────
                auto syncPos = [&]()
                    {
                        for (int dir = 0; dir <= 1; ++dir)
                        {
                            bool isV = (dir == 1);
                            float mn = FLT_MAX, mx = -FLT_MAX;
                            for (auto& a : axes)
                                if (a.vertical == isV)
                                {
                                    mn = std::min(mn, a.realPos); mx = std::max(mx, a.realPos);
                                }
                            if (mn == FLT_MAX) continue;
                            float range = mx - mn;
                            for (auto& a : axes)
                            {
                                if (a.vertical != isV) continue;
                                a.pos = (range > 1e-6f)
                                    ? 0.05f + (a.realPos - mn) / range * 0.90f
                                    : 0.5f;
                            }
                        }
                    };

                // ── 拖拽后 pos → realPos ────────────────────────────────────
                auto syncRealFromPos = [&](int idx)
                    {
                        bool isV = axes[idx].vertical;
                        float mn = FLT_MAX, mx = -FLT_MAX, pmin = FLT_MAX, pmax = -FLT_MAX;
                        for (auto& a : axes)
                            if (a.vertical == isV)
                            {
                                mn = std::min(mn, a.realPos); mx = std::max(mx, a.realPos);
                                pmin = std::min(pmin, a.pos);  pmax = std::max(pmax, a.pos);
                            }
                        float pr = pmax - pmin, rr = mx - mn;
                        if (pr > 1e-6f && rr > 1e-6f)
                            axes[idx].realPos = mn + (axes[idx].pos - pmin) / pr * rr;
                    };

                // ── 纵/横轴世界坐标范围 ─────────────────────────────────────
                auto calcRange = [&](float& vMin, float& vMax, float& hMin, float& hMax)
                    {
                        vMin = FLT_MAX; vMax = -FLT_MAX;
                        hMin = FLT_MAX; hMax = -FLT_MAX;
                        for (auto& a : axes)
                        {
                            if (a.vertical) { vMin = std::min(vMin, a.realPos); vMax = std::max(vMax, a.realPos); }
                            else { hMin = std::min(hMin, a.realPos); hMax = std::max(hMax, a.realPos); }
                        }
                        if (vMin == FLT_MAX) { vMin = 0.0f; vMax = 10.0f; }
                        if (hMin == FLT_MAX) { hMin = 0.0f; hMax = 10.0f; }
                    };

                // ── 顶部工具条 ────────────────────────────────────────────────
                ImGui::Text("方向："); ImGui::SameLine();
                ImGui::RadioButton("纵向", &addVertical, 1); ImGui::SameLine();
                ImGui::RadioButton("横向", &addVertical, 0); ImGui::SameLine();
                if (ImGui::Button("添加轴"))
                {
                    Axis a;
                    a.vertical = (addVertical != 0);
                    a.visible = true;
                    float maxR = 0.0f;
                    for (auto& x : axes) if (x.vertical == a.vertical) maxR = std::max(maxR, x.realPos);
                    a.realPos = axes.empty() ? 0.0f : maxR + 3.0f;
                    a.pos = 0.5f;
                    std::ostringstream ss; ss << "轴" << axisCounter++;
                    a.name = ss.str();
                    axes.push_back(std::move(a));
                    syncPos();
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100.0f);
                ImGui::InputFloat("出头(m)", &axisExtend, 0.5f, 1.0f, "%.1f");
                axisExtend = std::max(0.0f, axisExtend);

                // 原点坐标
                ImGui::Text("原点："); ImGui::SameLine();
                ImGui::SetNextItemWidth(80.0f);
                ImGui::InputFloat("X##ox", &gridOrigin.x, 1.0f, 10.0f, "%.2f");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(80.0f);
                ImGui::InputFloat("Y##oy", &gridOrigin.y, 1.0f, 10.0f, "%.2f");

                ImGui::Columns(2, "axis_cols");

                // ════════════════════════════════════════════════════════════
                // 左侧画布
                // ════════════════════════════════════════════════════════════
                ImGui::BeginChild("AxisCanvas", ImVec2(0, 400), true);
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
                            for (int i = 0; i < (int)axes.size(); ++i)
                                if (axes[i].vertical == vert && axes[i].visible)
                                    idx.push_back(i);
                            std::sort(idx.begin(), idx.end(),
                                [&](int a, int b) { return axes[a].pos < axes[b].pos; });
                            return idx;
                        };

                    ImU32 lineCol = IM_COL32(200, 200, 50, 255);
                    ImU32 extCol = IM_COL32(200, 200, 50, 120); // 出头部分（淡）
                    ImU32 bubbleBg = IM_COL32(30, 30, 30, 230);
                    ImU32 bubbleFg = IM_COL32(220, 200, 60, 255);
                    ImU32 dimCol = IM_COL32(100, 220, 255, 220);
                    ImU32 tickCol = IM_COL32(100, 220, 255, 140);

                    // ── 辅助：画轴号圆泡 ──────────────────────────────────
                    auto drawBubble = [&](float cx, float cy, const std::string& name)
                        {
                            dl->AddCircleFilled(ImVec2(cx, cy), BUBBLE_R, bubbleBg);
                            dl->AddCircle(ImVec2(cx, cy), BUBBLE_R, bubbleFg, 0, 1.5f);
                            ImVec2 tsz = ImGui::CalcTextSize(name.c_str());
                            dl->AddText(ImVec2(cx - tsz.x * 0.5f, cy - tsz.y * 0.5f), bubbleFg, name.c_str());
                        };

                    // ── 绘制轴线 + 出头 + 圆泡 ───────────────────────────
                    for (size_t i = 0; i < axes.size(); ++i)
                    {
                        if (!axes[i].visible) continue;

                        if (axes[i].vertical)
                        {
                            float x = gL + axes[i].pos * gW;

                            // 主体：网格范围内（实线）
                            dl->AddLine(ImVec2(x, gT), ImVec2(x, gB), lineCol, 1.5f);
                            // 出头：超出网格到 margin 区（淡线）
                            dl->AddLine(ImVec2(x, CP.y + MARGIN * 0.15f), ImVec2(x, gT), extCol, 1.5f);
                            dl->AddLine(ImVec2(x, gB), ImVec2(x, CP.y + CS.y - MARGIN * 0.15f), extCol, 1.5f);
                            // 圆泡
                            drawBubble(x, CP.y + BUBBLE_R + 2.0f, axes[i].name);
                            drawBubble(x, CP.y + CS.y - BUBBLE_R - 2.0f, axes[i].name);

                            // 拖拽命中检测
                            if (hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                                if (std::abs(mouse.x - x) < 6.0f && mouse.y > gT && mouse.y < gB)
                                    dragging = (int)i;
                        }
                        else
                        {
                            float y = gT + axes[i].pos * gH;

                            dl->AddLine(ImVec2(gL, y), ImVec2(gR, y), lineCol, 1.5f);
                            dl->AddLine(ImVec2(CP.x + MARGIN * 0.15f, y), ImVec2(gL, y), extCol, 1.5f);
                            dl->AddLine(ImVec2(gR, y), ImVec2(CP.x + CS.x - MARGIN * 0.15f, y), extCol, 1.5f);
                            drawBubble(CP.x + BUBBLE_R + 2.0f, y, axes[i].name);
                            drawBubble(CP.x + CS.x - BUBBLE_R - 2.0f, y, axes[i].name);

                            if (hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                                if (std::abs(mouse.y - y) < 6.0f && mouse.x > gL && mouse.x < gR)
                                    dragging = (int)i;
                        }
                    }

                    // ── 距离标注（相邻同向轴之间）──────────────────────────
                    // 纵向：尺寸线画在网格顶部内侧
                    {
                        auto idx = sorted(true);
                        for (int k = 1; k < (int)idx.size(); ++k)
                        {
                            int  a = idx[k - 1], b = idx[k];
                            float dist = std::abs(axes[b].realPos - axes[a].realPos);
                            float xa = gL + axes[a].pos * gW;
                            float xb = gL + axes[b].pos * gW;
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
                            float dist = std::abs(axes[b].realPos - axes[a].realPos);
                            float ya = gT + axes[a].pos * gH;
                            float yb = gT + axes[b].pos * gH;
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
                    if (dragging >= 0 && dragging < (int)axes.size() &&
                        ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        if (axes[dragging].vertical)
                            axes[dragging].pos = std::clamp((mouse.x - gL) / gW, 0.0f, 1.0f);
                        else
                            axes[dragging].pos = std::clamp((mouse.y - gT) / gH, 0.0f, 1.0f);
                        syncRealFromPos(dragging);
                    }
                    if (dragging >= 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                        dragging = -1;

                    ImGui::EndChild();
                }

                ImGui::NextColumn();

                // ════════════════════════════════════════════════════════════
                // 右侧轴表
                // ════════════════════════════════════════════════════════════
                ImGui::BeginChild("AxisTable", ImVec2(0, 400), false);
                {
                    ImGui::Columns(5, "atc");
                    ImGui::SetColumnWidth(0, 60); ImGui::SetColumnWidth(1, 50);
                    ImGui::SetColumnWidth(2, 80); ImGui::SetColumnWidth(3, 40);
                    ImGui::Text("名称");    ImGui::NextColumn();
                    ImGui::Text("方向");    ImGui::NextColumn();
                    ImGui::Text("位置(m)"); ImGui::NextColumn();
                    ImGui::Text("显示");    ImGui::NextColumn();
                    ImGui::Text("操作");    ImGui::NextColumn();
                    ImGui::Separator();

                    bool needSync = false;
                    for (int i = 0; i < (int)axes.size(); ++i)
                    {
                        ImGui::TextUnformatted(axes[i].name.c_str()); ImGui::NextColumn();
                        ImGui::TextUnformatted(axes[i].vertical ? "纵" : "横"); ImGui::NextColumn();
                        ImGui::PushID(i);
                        ImGui::PushItemWidth(-1);
                        if (ImGui::InputFloat("##p", &axes[i].realPos, 0.5f, 1.0f, "%.2f")) needSync = true;
                        ImGui::PopItemWidth(); ImGui::NextColumn();
                        if (ImGui::Checkbox("##v", &axes[i].visible)) {}
                        ImGui::NextColumn();
                        if (ImGui::SmallButton("中")) { axes[i].realPos = 0.0f; needSync = true; }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("X"))
                        {
                            axes.erase(axes.begin() + i);
                            ImGui::PopID(); needSync = true; break;
                        }
                        ImGui::NextColumn();
                        ImGui::PopID();
                    }

                    // 间距一览
                    if (!axes.empty())
                    {
                        ImGui::Columns(1);
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "间距一览");

                        std::vector<int> vIdx, hIdx;
                        for (int i = 0; i < (int)axes.size(); ++i)
                            (axes[i].vertical ? vIdx : hIdx).push_back(i);
                        auto cmp = [&](int a, int b) { return axes[a].realPos < axes[b].realPos; };
                        std::sort(vIdx.begin(), vIdx.end(), cmp);
                        std::sort(hIdx.begin(), hIdx.end(), cmp);

                        if (vIdx.size() >= 2) {
                            ImGui::TextDisabled("纵轴：");
                            for (int k = 1; k < (int)vIdx.size(); ++k)
                                ImGui::Text("  %s → %s : %.2f m",
                                    axes[vIdx[k - 1]].name.c_str(), axes[vIdx[k]].name.c_str(),
                                    axes[vIdx[k]].realPos - axes[vIdx[k - 1]].realPos);
                        }
                        if (hIdx.size() >= 2) {
                            ImGui::TextDisabled("横轴：");
                            for (int k = 1; k < (int)hIdx.size(); ++k)
                                ImGui::Text("  %s → %s : %.2f m",
                                    axes[hIdx[k - 1]].name.c_str(), axes[hIdx[k]].name.c_str(),
                                    axes[hIdx[k]].realPos - axes[hIdx[k - 1]].realPos);
                        }
                    }

                    if (needSync) syncPos();
                }
                ImGui::EndChild();

                ImGui::Columns(1);
                ImGui::Separator();

                // ── 确定 / 取消 ───────────────────────────────────────────
                if (ImGui::Button("确定"))
                {
                    float vMin, vMax, hMin, hMax;
                    calcRange(vMin, vMax, hMin, hMax);

                    auto& scene = document.GetScene();
                    for (auto& a : axes)
                    {
                        if (!a.visible) continue;

                        // realPos 减去各方向最小值，使最左纵轴/最底横轴对齐到 gridOrigin
                        // 世界坐标 = gridOrigin + (realPos - min)
                        XMFLOAT3 start, end;
                        if (a.vertical)
                        {
                            float wx = gridOrigin.x + (a.realPos - vMin);
                            float wyMin = gridOrigin.y + 0.0f - axisExtend; // hMin - hMin = 0
                            float wyMax = gridOrigin.y + (hMax - hMin) + axisExtend;
                            start = { wx, wyMin, 0.0f };
                            end = { wx, wyMax, 0.0f };
                        }
                        else
                        {
                            float wy = gridOrigin.y + (a.realPos - hMin);
                            float wxMin = gridOrigin.x + 0.0f - axisExtend;
                            float wxMax = gridOrigin.x + (vMax - vMin) + axisExtend;
                            start = { wxMin, wy, 0.0f };
                            end = { wxMax, wy, 0.0f };
                        }

                        if (!MiniCAD::Line(start, end, true).IsValid()) continue;
                        scene.AddEntity(std::make_unique<LineEntity>(scene.NextObjectID(), start, end));
                    }
                    showAxisWindow = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("取消")) showAxisWindow = false;
            }
            ImGui::End();
        }
        ImGui::End();
    }
}
