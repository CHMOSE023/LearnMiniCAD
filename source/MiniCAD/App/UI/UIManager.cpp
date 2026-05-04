#include "UIManager.h"
#include <imgui.h>
#include "imgui_internal.h"
#include <memory>
#include "App/Document/Document.h"
#include "App/Document/DocumentManager.h"
#include "pch.h"

namespace MiniCAD
{ 
    namespace
    {
        constexpr float kToolbarHeight   = 38.f;   // 工具栏高度
        constexpr float kToolBtnSize     = 32.f;   // 工具按钮尺寸
        constexpr float kStatusBarHeight = 26.f;   // 状态栏高度

        // 工具元数据
        struct ToolMeta { Tool id; const char* label; const char* tooltip; };
        constexpr ToolMeta kTools[] =
        {
            { Tool::Select,    "↖",  "选择 (Esc)"      },
            { Tool::Line,      "╱",  "直线 (L)"        },
            { Tool::Circle,    "○",  "圆 (C)"          },
            { Tool::Rectangle, "□",  "矩形 (R)"        },
            { Tool::Arc,       "⌒",  "圆弧 (A)"        },
            { Tool::Pan,       "✥",  "平移 (Space)"    },
            { Tool::Zoom,      "⊕",  "缩放 (Z)"        },
        };
    }

    // ── 工具函数 ────────────────────────────────────────────
    static ImVec2 RectCenter(ImVec2 min, ImVec2 size)
    {
        return ImVec2(min.x + size.x * 0.5f, min.y + size.y * 0.5f);
    }

    static void DrawMinimizeIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col)
    {
        float half = size * 0.5f;
        dl->AddLine(
            ImVec2(center.x - half, center.y ),
            ImVec2(center.x + half, center.y ),
            col, 1.2f
        );
    }

    static void DrawMaximizeIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col)
    {
        float half = size * 0.5f;
        dl->AddRect(
            ImVec2(center.x - half, center.y - half),
            ImVec2(center.x + half, center.y + half),
            col, 0.f, 0, 1.2f
        );
    }

    static void DrawRestoreIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col)
    {
        float h   = size * 0.40f;
        float off = size * 0.10f;

        // ── 后面的矩形（右上角偏移，只画三条边，左下角被前矩形遮住不画）
        ImVec2 b0 = ImVec2(center.x - h + off, center.y - h - off); // 左上
        ImVec2 b1 = ImVec2(center.x + h + off, center.y - h - off); // 右上
        ImVec2 b2 = ImVec2(center.x + h + off, center.y + h - off); // 右下
   
        dl->AddLine(b0, b1, col, 1.2f);   // 上边
        dl->AddLine(b1, b2, col, 1.2f);   // 右边 

        // ── 前面的矩形（左下角偏移，完整画四条边）
        ImVec2 f0 = ImVec2(center.x - h - off, center.y - h + off); // 左上
        ImVec2 f1 = ImVec2(center.x + h - off, center.y - h + off); // 右上
        ImVec2 f2 = ImVec2(center.x + h - off, center.y + h + off); // 右下
        ImVec2 f3 = ImVec2(center.x - h - off, center.y + h + off); // 左下

        dl->AddLine(f0, f1, col, 1.2f);
        dl->AddLine(f1, f2, col, 1.2f);
        dl->AddLine(f2, f3, col, 1.2f);
        dl->AddLine(f3, f0, col, 1.2f);
    }

    static void DrawCloseIcon(ImDrawList* dl, ImVec2 center, float size, ImU32 col)
    {
        // 像素对齐，消除亚像素偏移导致的不对称
        float cx = center.x;
        float cy = center.y;
         
        dl->AddLine(ImVec2(cx - 5.0, cy - 5.0), ImVec2(cx + 5.0, cy + 5.4), col, 1.2f);
        dl->AddLine(ImVec2(cx + 5.0, cy - 5.4), ImVec2(cx - 5.0, cy + 5.0), col, 1.2f);
    }
     
    bool UIManager::Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        m_hwnd = hwnd; 

        m_imgui = std::make_unique<ImGuiLayer>();
        if (!m_imgui->Init(hwnd, device, context))
            return false;  

        ImGuiIO& io = ImGui::GetIO();   
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 16.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());  
        ImGui::StyleColorsDark();
        return true;
    }     
    void UIManager::Shutdown()  { m_imgui->Shutdown(); }
    void UIManager::BeginFrame(){ m_imgui->Begin(); }
    void UIManager::EndFrame()  { m_imgui->End(); }
     
    void UIManager::Render(DocumentManager& dm)
    {    
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar            |
                                 ImGuiWindowFlags_NoCollapse            |
                                 ImGuiWindowFlags_NoResize              |
                                 ImGuiWindowFlags_NoMove                |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus |
                                 ImGuiWindowFlags_NoNavFocus            |
                                 ImGuiWindowFlags_NoBackground          | 
                                 ImGuiWindowFlags_MenuBar               |
                                 ImGuiWindowFlags_NoScrollbar           |      // 不显示滚动条
                                 ImGuiWindowFlags_NoScrollWithMouse     ;      // 禁止滚动行为
          
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0, 6.f)); // 菜单栏高度 
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.f, 0.f, 0.f, 0.f)); 
        ImGui::Begin("MiniCAD", nullptr, flags);
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(3);
        // ── 1. 菜单栏（含最小化/最大化/关闭） ──────────────────
        DrawMenubar(dm);

        // ── 2. 工具栏 ────────────────────────────────────────────
        DrawToolbar(dm);

        // ── 3. 绘图区（剩余高度 - 状态栏） ──────────────────────
        { 
            ImGui::BeginChild("##DocArea", ImVec2(0, -kStatusBarHeight),  false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
            DrawDocumentTabs(dm);
            ImGui::EndChild();
        }

        // ── 4. 状态栏 ────────────────────────────────────────────
        DrawStatusBar(dm); 

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode); 
        ImGui::End();
    }
     
    void UIManager::DrawMenubar(DocumentManager& dm)
    { 
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 6.f)); // 菜单栏高度 
       
        if (!ImGui::BeginMenuBar())
        {
            ImGui::PopStyleVar(); 
            ImGui::PopStyleColor(); // 注意：即使没进去也要 Pop 
            return;
        }
       
        // ── 菜单项 ───────────────────────────────────────────────
        if (ImGui::BeginMenu("文件"))
        {
            if (ImGui::MenuItem("新建", "Ctrl+N")) {}
            if (ImGui::MenuItem("打开", "Ctrl+O")) {}
            if (ImGui::MenuItem("保存", "Ctrl+S")) {}
            if (ImGui::MenuItem("另存为", "Ctrl+Shift+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("退出", "Alt+F4"))
                PostMessage(m_hwnd, WM_CLOSE, 0, 0);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("编辑"))
        {
            if (ImGui::MenuItem("撤销", "Ctrl+Z")) {}
            if (ImGui::MenuItem("重做", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("复制", "Ctrl+C")) {}
            if (ImGui::MenuItem("粘贴", "Ctrl+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("视图"))
        {
            static bool showGrid = true, showAxis = true;
            ImGui::MenuItem("显示网格", nullptr, &showGrid);
            ImGui::MenuItem("显示坐标轴", nullptr, &showAxis);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("帮助"))
        {
            if (ImGui::MenuItem("关于")) {}
            ImGui::EndMenu();
        }

        // ── 右侧窗口控制按钮 ─────────────────────────────────────
        {
            const float btnW = 32.f;
            const float gap = 8.f;
            const float totalW = btnW * 3.f + gap * 3.f + ImGui::GetStyle().WindowPadding.x;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalW);

            float   buttonsLocalX = ImGui::GetWindowWidth() - totalW;
            ImVec2  screenPos = ImGui::GetWindowPos();
            m_captionButtonsScreenX = screenPos.x + buttonsLocalX;

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 1.f, 1.f, 0.15f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.f, 1.f, 1.f, 0.15f));

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImU32       iconCol = IM_COL32(255, 255, 255, 255);
            const float iconSize = 10.f;

            // ── 最小化 ───────────────────────────────────────────
            ImGui::Button("##min", ImVec2(btnW, 0.f));
            DrawMinimizeIcon(dl, RectCenter(ImGui::GetItemRectMin(), ImGui::GetItemRectSize()), iconSize, iconCol);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("最小化");
            if (ImGui::IsItemClicked()) ShowWindow(m_hwnd, SW_MINIMIZE);

            ImGui::SameLine(0.f, gap);

            // ── 最大化 / 还原 ────────────────────────────────────
            bool maximized = IsZoomed(m_hwnd);
            ImGui::Button("##max", ImVec2(btnW, 0.f));
            ImVec2 maxCenter = RectCenter(ImGui::GetItemRectMin(), ImGui::GetItemRectSize());
            if (maximized) DrawRestoreIcon (dl, maxCenter, iconSize, iconCol);
            else           DrawMaximizeIcon(dl, maxCenter, iconSize, iconCol);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip(maximized ? "还原" : "最大化");
            if (ImGui::IsItemClicked()) ShowWindow(m_hwnd, maximized ? SW_RESTORE : SW_MAXIMIZE);

            ImGui::SameLine(0.f, gap);

            // ── 关闭 ─────────────────────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.15f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.65f, 0.05f, 0.05f, 1.f));
            ImGui::Button("##close", ImVec2(btnW, 0.f));
            DrawCloseIcon(dl, RectCenter(ImGui::GetItemRectMin(), ImGui::GetItemRectSize()), iconSize, iconCol);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("关闭");
            if (ImGui::IsItemClicked()) PostMessage(m_hwnd, WM_CLOSE, 0, 0);
            ImGui::PopStyleColor(2);

            ImGui::PopStyleColor(3);
        }

        ImGui::EndMenuBar();   
        ImGui::PopStyleVar();
    }
     
    void UIManager::DrawToolbar(DocumentManager& dm)
    {
        ImGui::BeginChild("##Toolbar", ImVec2(0.f, kToolbarHeight), false);

        float leftPadding = 3.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + leftPadding);

        const ImVec2 btnSize(kToolBtnSize, kToolBtnSize);

        // ===== 1️ 计算背景区域 =====
        ImVec2 start = ImGui::GetCursorScreenPos();
        float padding = 3.0f;

        float bgHeight = kToolBtnSize + padding * 2.0f;
        float bgWidth = ImGui::GetContentRegionAvail().x-6;

        // 占位（关键）
        ImGui::Dummy(ImVec2(bgWidth, bgHeight));

        // ===== 2️ 画背景（不会盖按钮）=====
        ImDrawList* draw = ImGui::GetWindowDrawList();

        draw->AddRectFilled(start, ImVec2(start.x + bgWidth, start.y + bgHeight), IM_COL32(40, 40, 45, 255), 6.0f);

        // ===== 3️ 回到起点，开始画按钮 =====
        ImGui::SetCursorScreenPos(ImVec2(start.x + padding, start.y + padding));

        ImGui::BeginGroup();

        // 按钮样式（透明底）
        ImVec4 btn = ImVec4(0, 0, 0, 0);
        ImVec4 hover = ImVec4(0.25f, 0.25f, 0.30f, 0.8f);
        ImVec4 activeC = ImVec4(0.35f, 0.55f, 0.85f, 0.9f);

        for (auto& meta : kTools)
        {
            bool active = (m_activeTool == meta.id);

            ImGui::PushStyleColor(ImGuiCol_Button, btn);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, active ? activeC : hover);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

            ImGui::PushID(static_cast<int>(meta.id));

            if (ImGui::Button(meta.label, btnSize))
                m_activeTool = meta.id;

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", meta.tooltip);

            ImGui::PopID();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            // 分隔线
            if (meta.id == Tool::Select || meta.id == Tool::Arc)
            {
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::SameLine();
            }
        }

        // ===== 4️⃣ 右侧 Undo / Redo =====
        float rightOffset = bgWidth - (btnSize.x * 2.0f + padding * 2.0f + 6.0f);
        ImGui::SameLine(rightOffset);

        ImGui::PushStyleColor(ImGuiCol_Button, btn);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, hover);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        if (ImGui::Button("↩", btnSize)) { /* TODO: Undo */ }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("撤销 (Ctrl+Z)");

        ImGui::SameLine(0.f, 4.f);

        if (ImGui::Button("↪", btnSize)) { /* TODO: Redo */ }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("重做 (Ctrl+Y)");

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::EndGroup();

        ImGui::EndChild();
    }
     
    void UIManager::DrawDocumentTabs(DocumentManager& dm)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (!ImGui::BeginTabBar("MiniCAD##Main"))
        {
            ImGui::PopStyleVar();
            return;   //  不再错误调用外层 End()
        }

        auto& docs  = dm.GetAll();
        Document* active = dm.GetActive();

        for (auto& docPtr : docs)
        {
            Document* doc  = docPtr.get();
            bool      open = true;

            ImGui::PushID(doc);

            std::string label = doc->GetName();
            if (doc->IsDirty()) label += " *";

            if (ImGui::BeginTabItem(label.c_str(), &open))
            {
                if (doc != active)
                    dm.SetActive(doc);

                // 去掉内边距
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                ImVec2 size = ImGui::GetContentRegionAvail();
                auto   srv  = doc->GetViewport().GetRenderTarget().GetSRV();
                ImGui::Image(srv, size);

                ImGui::PopStyleVar(2);
                if (doc == dm.GetActive())
                {
                    m_docImageState.Size = size;
                    m_docImageState.Min  = ImGui::GetItemRectMin();
                    m_docImageState.Max  = ImGui::GetItemRectMax();

                    ImVec2 mp = ImGui::GetMousePos();
                    if (!ImGui::IsMousePosValid(&mp)) mp = ImVec2(0.f, 0.f);
                    m_docImageState.MousePos = mp;

                    bool hov = ImGui::IsItemHovered();
                    m_docImageState.Hovered = hov;

                    if (hov)
                    {
                        // 隐藏鼠标
                        ImGui::SetMouseCursor(ImGuiMouseCursor_None);

                        ImVec2 local {
                            ImClamp(mp.x - m_docImageState.Min.x, 0.f, size.x),
                            ImClamp(mp.y - m_docImageState.Min.y, 0.f, size.y)
                        };
                        m_docImageState.Delta = { local.x - m_lastLocal.x,
                                                  local.y - m_lastLocal.y };
                        m_docImageState.Local = local;
                        m_lastLocal = local;
                    }
                    else
                    {
                        m_docImageState.Delta = ImVec2(0.f, 0.f);
                    }
                }

                ImGui::EndTabItem();
            }

            ImGui::PopID();

            if (!open)
            {
                dm.Close(doc);
                if (doc == active) dm.SetActive(nullptr);
                break;
            }
        }

        ImGui::PopStyleVar();
        ImGui::EndTabBar();
    } 

    void UIManager::DrawStatusBar(DocumentManager& dm)
    {
        ImGuiStyle& s = ImGui::GetStyle();

        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.f, 3.f));
        //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(8.f, 2.f));
        //ImGui::PushStyleColor(ImGuiCol_ChildBg,          ImVec4(0.15f, 0.15f, 0.15f, 1.f));

        ImGui::BeginChild("##StatusBar",
                          ImVec2(0.f, kStatusBarHeight),
                          false,
                          ImGuiWindowFlags_NoScrollbar);

        // ── 当前工具 ─────────────────────────────────────────────
        const char* toolNames[] = {
            "选择", "直线", "圆", "矩形", "圆弧", "平移", "缩放"
        };
        ImGui::TextDisabled("工具:");
        ImGui::SameLine();
        ImGui::TextUnformatted(toolNames[static_cast<int>(m_activeTool)]);

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        // ── 鼠标坐标 ─────────────────────────────────────────────
        const auto& state = m_docImageState;
        ImGui::TextDisabled("坐标:");
        ImGui::SameLine();
        if (state.Hovered)
            ImGui::Text("X: %.1f  Y: %.1f", state.Local.x, state.Local.y);
        else
            ImGui::TextDisabled("---");

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        // ── 当前文档 ─────────────────────────────────────────────
        Document* active = dm.GetActive();
        ImGui::TextDisabled("文档:");
        ImGui::SameLine();
        if (active)
        {
            ImGui::TextUnformatted(active->GetName().c_str());
            if (active->IsDirty())
            {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.8f, 0.2f, 1.f));
                ImGui::TextUnformatted("● 未保存");
                ImGui::PopStyleColor();
            }
        }
        else
        {
            ImGui::TextDisabled("无");
        }

        // ── 右侧：文档数量 ───────────────────────────────────────
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "共 %zu 个文档",
                     dm.GetAll().size());
            float tw = ImGui::CalcTextSize(buf).x;
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - tw - s.WindowPadding.x);
            ImGui::TextDisabled("%s", buf);
        }

        ImGui::EndChild();
        //ImGui::PopStyleColor();
        //ImGui::PopStyleVar(2);
    }

}  
