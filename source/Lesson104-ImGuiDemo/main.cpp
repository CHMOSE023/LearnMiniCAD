#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h> 
#include "imgui_internal.h" 
#include <d3dcompiler.h> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  
#pragma comment(lib,"d3dcompiler.lib")
#define IM_PI 3.1415926
// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
 
ImTextureID toolIcons[8];

// ── 全局状态 ────────────────────────────   
ImTextureID LoadTextureFromFile(ID3D11Device* device, const char* path);
void InitToolIcons();
void FreeToolIcons();
void RenderFrameDrawList();
void RenderFrameDrawImage();
void RenderLaylout();
void BeginDockSpace();

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 每个工具的绘制函数类型
using DrawIconFn = void(*)(ImDrawList*, ImVec2 center, float size, ImU32 color);

// ---- 各工具图标绘制函数 ---- 

void DrawIcon_Select(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    // 鼠标箭头形状
    ImVec2 pts[] = {
        { c.x - s * 0.3f, c.y - s * 0.5f },
        { c.x - s * 0.3f, c.y + s * 0.3f },
        { c.x - s * 0.0f, c.y + s * 0.1f },
        { c.x + s * 0.2f, c.y + s * 0.5f },
        { c.x + s * 0.35f,c.y + s * 0.4f },
        { c.x + s * 0.15f,c.y + s * 0.0f },
        { c.x + s * 0.4f, c.y + s * 0.0f },
    };
    dl->AddPolyline(pts, 7, col, ImDrawFlags_Closed, 1.5f);
}

void DrawIcon_Line(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    dl->AddLine(
        { c.x - s * 0.5f, c.y + s * 0.5f },
        { c.x + s * 0.5f, c.y - s * 0.5f },
        col, 1.5f
    );
    dl->AddCircleFilled({ c.x - s * 0.5f, c.y + s * 0.5f }, 2.5f, col);
    dl->AddCircleFilled({ c.x + s * 0.5f, c.y - s * 0.5f }, 2.5f, col);
}

void DrawIcon_Arc(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    dl->PathArcTo(c, s * 0.5f, IM_PI * 0.8f, IM_PI * 2.2f, 32);
    dl->PathStroke(col, ImDrawFlags_None, 1.5f);
}

void DrawIcon_Circle(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    dl->AddCircle(c, s * 0.45f, col, 32, 1.5f);
    dl->AddCircleFilled(c, 2.0f, col); // 圆心点
}

void DrawIcon_Rect(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    dl->AddRect(
        { c.x - s * 0.45f, c.y - s * 0.35f },
        { c.x + s * 0.45f, c.y + s * 0.35f },
        col, 2.0f, 0, 1.5f
    );
}

void DrawIcon_Edit(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    // 铅笔形状
    ImVec2 pts[] = {
        { c.x - s * 0.4f, c.y + s * 0.5f },
        { c.x + s * 0.3f, c.y - s * 0.3f },
        { c.x + s * 0.5f, c.y - s * 0.1f },
        { c.x - s * 0.2f, c.y + s * 0.5f },
    };
    dl->AddPolyline(pts, 4, col, ImDrawFlags_Closed, 1.5f);
    dl->AddLine(
        { c.x - s * 0.4f, c.y + s * 0.5f },
        { c.x - s * 0.5f, c.y + s * 0.5f },
        col, 1.5f
    );
}

void DrawIcon_Move(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    // 四向箭头
    float a = s * 0.45f, h = s * 0.2f;
    // 上
    dl->AddLine(c, { c.x,        c.y - a }, col, 1.5f);
    dl->AddTriangleFilled({ c.x - h * 0.6f, c.y - a }, { c.x + h * 0.6f, c.y - a }, { c.x, c.y - a - h }, col);
    // 下
    dl->AddLine(c, { c.x,        c.y + a }, col, 1.5f);
    dl->AddTriangleFilled({ c.x - h * 0.6f, c.y + a }, { c.x + h * 0.6f, c.y + a }, { c.x, c.y + a + h }, col);
    // 左
    dl->AddLine(c, { c.x - a,    c.y }, col, 1.5f);
    dl->AddTriangleFilled({ c.x - a, c.y - h * 0.6f }, { c.x - a, c.y + h * 0.6f }, { c.x - a - h, c.y }, col);
    // 右
    dl->AddLine(c, { c.x + a,    c.y }, col, 1.5f);
    dl->AddTriangleFilled({ c.x + a, c.y - h * 0.6f }, { c.x + a, c.y + h * 0.6f }, { c.x + a + h, c.y }, col);
}

void DrawIcon_Delete(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    // 垃圾桶
    float w = s * 0.5f, h = s * 0.55f;
    // 桶身
    dl->AddRect({ c.x - w * 0.7f, c.y - h * 0.3f }, { c.x + w * 0.7f, c.y + h * 0.7f }, col, 1.0f, 0, 1.5f);
    // 盖子
    dl->AddLine({ c.x - w, c.y - h * 0.3f }, { c.x + w, c.y - h * 0.3f }, col, 1.5f);
    dl->AddRect({ c.x - w * 0.4f, c.y - h * 0.6f }, { c.x + w * 0.4f, c.y - h * 0.3f }, col, 0, 0, 1.5f);
    // 桶内竖线
    dl->AddLine({ c.x,            c.y - h * 0.1f }, { c.x,            c.y + h * 0.5f }, col, 1.2f);
    dl->AddLine({ c.x - w * 0.3f, c.y - h * 0.1f }, { c.x - w * 0.3f, c.y + h * 0.5f }, col, 1.2f);
    dl->AddLine({ c.x + w * 0.3f, c.y - h * 0.1f }, { c.x + w * 0.3f, c.y + h * 0.5f }, col, 1.2f);
}
 
int main(int, char**)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"LearnMiniCAD ImGui", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_HIDE);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;          
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       

    io.ConfigViewportsNoAutoMerge = true;

    // 中文
    io.Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/msyh.ttc",
        16.0f,
        nullptr,
        io.Fonts->GetGlyphRangesChineseFull()
    );

    ImGui::StyleColorsDark(); 
     
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        
    style.FontScaleDpi = main_scale;    
    style.WindowRounding = 12.0f;
    style.ChildRounding  = 6.0f;
    style.PopupRounding  = 6.0f;
    style.FrameRounding  = 6.0f;

    io.ConfigDpiScaleFonts = true;        
    io.ConfigDpiScaleViewports = true;     
  

    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    //{
    //    style.WindowRounding = 8.0f;
    //    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    //}
    // 


    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
        
    bool show_demo_window    = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
     
  



    InitToolIcons(); // 加载图标
    static bool showWindow1 = true;
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
          
         BeginDockSpace();
         RenderFrameDrawList();
         RenderFrameDrawImage();
         RenderLaylout();
      
        {  // 来个圆角

            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 12.0f;
            style.WindowBorderSize = 1.0f;
            style.WindowPadding = ImVec2(10, 10);

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
          
            //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            //ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.2f, 0.2f, 0.2f, 0.0f));

            ImGui::Begin("MainWindow1", &showWindow1,
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("文件"))
                {
                    ImGui::MenuItem("新建", "Ctrl+N");
                    ImGui::MenuItem("打开", "Ctrl+O");
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("编辑"))
                {
                    ImGui::MenuItem("撤销", "Ctrl+Z");
                    ImGui::EndMenu();
                }

                // 右侧 X
                float right = ImGui::GetWindowWidth();
                float btnWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2;

                ImGui::SameLine(right - btnWidth - 10);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0, 0, 0.2f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 0, 0, 0.4f));

                if (ImGui::Button("X"))
                    showWindow1 = false;

                ImGui::PopStyleColor(3);

                ImGui::EndMenuBar();
            }

            ImGui::Text("MainWindow1");

            ImGui::End();
             
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

ImTextureID LoadTextureFromFile(ID3D11Device* device, const char* path)
{
   // 用 stb_image 读取图片
    int w, h, ch;
    unsigned char* pixels = stbi_load(path, &w, &h, &ch, 4); // 强制 RGBA
    if (!pixels) return NULL;

    // 创建 D3D11 Texture2D
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = w;
    desc.Height           = h;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem     = pixels;
    initData.SysMemPitch = w * 4;

    ID3D11Texture2D* tex = nullptr;
    device->CreateTexture2D(&desc, &initData, &tex);
    stbi_image_free(pixels);

    if (!tex) return NULL;

    // 创建 SRV（Shader Resource View），这才是 ImGui 需要的
    ID3D11ShaderResourceView* srv = nullptr;
    device->CreateShaderResourceView(tex, nullptr, &srv);
    tex->Release(); // SRV 已持有引用，Texture 可以释放

    return (ImTextureID)srv;
}
 
void InitToolIcons()
{
    const char* iconPaths[] = {
        "icons/draw_rectangle.png",
        "icons/draw_line.png",
        "icons/draw_arc_cse.png",
        "icons/draw_circle_cr.png",
        "icons/draw_rectangle.png",
        "icons/draw_xline.png",
        "icons/draw_ray.png",
        "icons/draw_polygon.png"
    };

    for (int i = 0; i < 8; i++)
        toolIcons[i] = LoadTextureFromFile(g_pd3dDevice, iconPaths[i]);
}

void FreeToolIcons()
{
    for (int i = 0; i < 8; i++)
    {
        if (toolIcons[i])
        {
            ((ID3D11ShaderResourceView*)toolIcons[i])->Release();
            toolIcons[i] = NULL;
        }
    }
}

void RenderFrameDrawList()
{ 
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("ToolbarDrawList", nullptr, flags);

    static int activeTool = 0;

    const char* labels[]  = { "Select", "Line", "Arc", "Circle", "Rect", "Edit", "Move", "Delete" };
    DrawIconFn  drawFns[] = {
        DrawIcon_Select, DrawIcon_Line, DrawIcon_Arc,    DrawIcon_Circle,
        DrawIcon_Rect,   DrawIcon_Edit, DrawIcon_Move,   DrawIcon_Delete
    };
    int count = sizeof(labels) / sizeof(labels[0]);

    ImVec2 btnSize(36, 36);
    float  iconSize = 22.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int i = 0; i < count; i++)
    {
        ImGui::PushID(i);

        bool clicked = ImGui::InvisibleButton("##btn", btnSize);
        bool hovered = ImGui::IsItemHovered();
        bool held = ImGui::IsItemActive();
        bool active = (activeTool == i);

        if (clicked) activeTool = i;

        ImVec2 p = ImGui::GetItemRectMin();
        ImVec2 pMax = { p.x + btnSize.x, p.y + btnSize.y };
        ImVec2 center = { p.x + btnSize.x * 0.5f, p.y + btnSize.y * 0.5f };

        // ---- 背景 ----
        if (active)
            dl->AddRectFilled(p, pMax, IM_COL32(66, 150, 250, 60), 4.0f);
        else if (hovered || held)
            dl->AddRectFilled(p, pMax, IM_COL32(255, 255, 255, 25), 4.0f);

        // ---- 选中边框 ----
        if (active)
            dl->AddRect(p, pMax, IM_COL32(66, 150, 250, 200), 4.0f, 0, 1.5f);

        // ---- 图标颜色 ----
        ImU32 iconCol = active  ? IM_COL32(100, 180, 255, 255)  // 选中：亮蓝
                      : hovered ? IM_COL32(220, 220, 220, 255)  // 悬停：亮白
                      : held    ? IM_COL32(255, 255, 255, 255)  // 按住：纯白
                      : IM_COL32(160, 160, 160, 255);           // 默认：灰

        // ---- 绘制图标 ----
        drawFns[i](dl, center, iconSize, iconCol);

        ImGui::PopID();

        if (hovered)
            ImGui::SetTooltip("%s", labels[i]);

        ImGui::SameLine(0, 3);
    }

    ImGui::End();
}

void RenderFrameDrawImage()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings;

    // 去掉窗口背景
    ImGui::Begin("ToolbarDrawImage", nullptr, flags);

    static int activeTool = 0;

    const char* labels[] = { "Select", "Line", "Arc", "Circle", "Rect", "Edit", "Move", "Delete" };
    int count = sizeof(labels) / sizeof(labels[0]);

    // 去掉 ImageButton 自身的边框和背景
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));   // 默认透明
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.10f)); // 悬停：淡白
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.20f)); // 按下：稍亮

    for (int i = 0; i < count; i++)
    {
        bool active = (activeTool == i);

        if (active)
        {
            // 选中态：蓝色底 + 蓝色边框
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.30f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.26f, 0.59f, 0.98f, 0.80f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        }

        ImGui::PushID(i);
        if (ImGui::ImageButton("##icon", toolIcons[i], ImVec2(30, 30)))
            activeTool = i;
        ImGui::PopID();

        if (active)
        {
            ImGui::PopStyleVar();   // FrameBorderSize
            ImGui::PopStyleColor(2); // Button + Border
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", labels[i]);

        ImGui::SameLine(0, 3);
    }

    ImGui::PopStyleColor(3); // Button / ButtonHovered / ButtonActive
    ImGui::PopStyleVar(2);   // FramePadding / FrameRounding

    ImGui::End();
}
 
// 布局
void RenderLaylout()
{
    ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_MenuBar);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    // ======== 菜单栏 ========
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("文件"))
        {
            if (ImGui::MenuItem("新建", "Ctrl+N")) { /* ... */ }
            if (ImGui::MenuItem("打开", "Ctrl+O")) { /* ... */ }
            if (ImGui::MenuItem("保存", "Ctrl+S")) { /* ... */ }
            if (ImGui::MenuItem("另存为", "Ctrl+Shift+S")) { /* ... */ }
            ImGui::Separator();
            if (ImGui::MenuItem("退出", "Alt+F4")) { /* ... */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("编辑"))
        {
            if (ImGui::MenuItem("撤销", "Ctrl+Z")) { /* ... */ }
            if (ImGui::MenuItem("重做", "Ctrl+Y")) { /* ... */ }
            ImGui::Separator();
            if (ImGui::MenuItem("复制", "Ctrl+C")) { /* ... */ }
            if (ImGui::MenuItem("粘贴", "Ctrl+V")) { /* ... */ }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("视图"))
        {
            static bool showGrid = true;
            static bool showAxis = true;
            ImGui::MenuItem("显示网格", nullptr, &showGrid); // 带勾选状态
            ImGui::MenuItem("显示坐标轴", nullptr, &showAxis);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("帮助"))
        {
            if (ImGui::MenuItem("关于")) { /* ... */ }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::PopStyleColor();

    // 添加菜单栏可以吗？
    static float leftWidth = 220.0f;
    float totalWidth = ImGui::GetContentRegionAvail().x;
    float splitterWidth = 4.0f;
    float rightWidth = totalWidth - leftWidth - splitterWidth;


    // ======== 左侧 区域1 ========
    if (ImGui::BeginChild("##区域1", ImVec2(leftWidth, 0), true))
    {
        ImGui::Text("区域1");

        static float leftPanelWidth = 120.0f;
        float availW = ImGui::GetContentRegionAvail().x;
        float innerSplit = 4.0f;
        float rightPanelWidth = availW - leftPanelWidth - innerSplit;

        // 区域1 左列
        if (ImGui::BeginChild("##区域1_左", ImVec2(leftPanelWidth, 0), true))
        {
            ImGui::Text("区域1 - 左");

            if (ImGui::BeginChild("##区域1_左_上", ImVec2(0, 40), true))
            {
                ImGui::Text("区域1 - 左 - 上");
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::SameLine(0, 0);

        // 区域1 内部分隔条
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 1.0f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::Button("##splitter_inner", ImVec2(innerSplit, -1));
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        if (ImGui::IsItemActive())
        {
            leftPanelWidth += ImGui::GetIO().MouseDelta.x;
            leftPanelWidth = ImClamp(leftPanelWidth, 40.0f, availW - 40.0f);
        }
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        ImGui::SameLine(0, 0);

        // 区域1 右列
        if (ImGui::BeginChild("##区域1_右", ImVec2(rightPanelWidth, 0), true))
        {
            ImGui::Text("区域1 - 右");
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();

    ImGui::SameLine(0, 0);

    // ======== 主分隔条 ========
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 1.0f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::Button("##splitter_main", ImVec2(splitterWidth, -1));
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    if (ImGui::IsItemActive())
    {
        leftWidth += ImGui::GetIO().MouseDelta.x;
        leftWidth = ImClamp(leftWidth, 80.0f, totalWidth - 80.0f);
    }
    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    ImGui::SameLine(0, 0);

    // ======== 右侧 区域2 + TabBar ========
    if (ImGui::BeginChild("##区域2", ImVec2(rightWidth, 0), true))
    {
        if (ImGui::BeginTabBar("##tabs"))
        {
            if (ImGui::BeginTabItem("属性"))
            {
                ImGui::Text("属性面板内容");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("图层"))
            {
                ImGui::Text("图层面板内容");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("设置"))
            {
                ImGui::Text("设置面板内容");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();

    ImGui::End();

}

void BeginDockSpace()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;          // ❗自己不能被 dock 
      
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("DockSpaceRoot", nullptr, window_flags);

    ImGui::PopStyleVar(2);

    ImGui::Text("DockSpaceRootDockSpaceRootDockSpaceRootDockSpaceRootDockSpaceRootDockSpaceRootDockSpaceRoot"); 

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    ImGuiDockNodeFlags dock_flags =  ImGuiDockNodeFlags_PassthruCentralNode; // 中间透明（常用）

    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dock_flags);

    ImGui::End();
}
 
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

     IDXGIFactory* pSwapChainFactory;
    if (SUCCEEDED(g_pSwapChain->GetParent(IID_PPV_ARGS(&pSwapChainFactory))))
    {
        pSwapChainFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
        pSwapChainFactory->Release();
    }

    CreateRenderTarget();
    return true;
}

ID3DBlob* Compile(const char* src, const char* entry, const char* target)
{
    ID3DBlob* blob = nullptr;
    ID3DBlob* err = nullptr;
    HRESULT hr = D3DCompile(src, strlen(src), 0, 0, 0, entry, target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &blob, &err);
    if (FAILED(hr))
    {
        if (err)
        {
            OutputDebugStringA((char*)err->GetBufferPointer());
            err->Release();
        }
        return nullptr;
    }
    return blob;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
 
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth  = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
