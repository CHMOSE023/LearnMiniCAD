#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h> 
#include <cstdio>
#include <d3dcompiler.h> 
#include <algorithm>

#include <DirectXMath.h>
using namespace DirectX;
 
struct TVertex { float x, y, z, r, g, b, a; }; 

struct ViewportState
{
    // --- 图像信息 ---
    ImVec2 viewport_size;  // 显示尺寸（窗口中）
    ImVec2 viewport_min;   // 屏幕坐标：左上角
    ImVec2 viewport_max;   // 屏幕坐标：右下角

    // --- 输入 ---
    ImVec2 mouse_pos;   // 鼠标屏幕坐标
    ImVec2 mouse_local; // 相对图像坐标 

    float  wheel = 0;   // 滚轮
    ImVec2 delta;       // 偏移量
};

struct Camera2D
{
    float    zoom   = 1.0f;
    XMFLOAT2 offset = { 0.0f, 0.0f }; // 平移
};

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Offscreen(离屏渲染 核心三件套)
ID3D11Texture2D*                g_offTex  = nullptr;  // GPU 上真实存像素的“图片内存”
ID3D11RenderTargetView*         g_offRtv  = nullptr;  // 让 GPU “可以往这张纹理画东西”
ID3D11ShaderResourceView*       g_offSrv  = nullptr;  // 让 shader 可以读取这张纹理
/* 
   // 两者指向同一块 g_offTex 内存，但权限不同
   g_offRtv  // RenderTargetView   = "可写入" 视图  → 只有 RenderOffscreen 用它写
   g_offSrv  // ShaderResourceView = "只读"  视图  → ImGui::Image 用它读
*/
                                
// Triangle(三角形)        
ID3D11Buffer*                   g_vsConst = nullptr;  // 位置
ID3D11Buffer*                   g_vb      = nullptr;  // 顶点
ID3D11VertexShader*             g_vs      = nullptr;
ID3D11PixelShader*              g_ps      = nullptr;
ID3D11InputLayout*              g_layout  = nullptr;


XMMATRIX                        g_viewProj = XMMatrixIdentity();// 投影 
Camera2D                        g_cam      = {};                // 相机
int  g_width  = 1800;
int  g_height = 1600;


void InitOffscreen();
void RenderOffscreen();
void RenderViewport();
void UpdateViewProj(ViewportState viewportState); // 更新投影矩阵
void ResizeOffscreen(int w, int h);               // 动态重建纹理

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
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

    //  Lesson024 -> 1.准备数据
    InitOffscreen();

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;          
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    

    io.ConfigWindowsMoveFromTitleBarOnly = true; // 只能通过标题栏

    ImGui::StyleColorsDark(); 
     
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        
    style.FontScaleDpi = main_scale;       
    io.ConfigDpiScaleFonts = true;        
    io.ConfigDpiScaleViewports = true;     


    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
     
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
        
    bool show_demo_window    = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
         
        RenderViewport();    // 呈现视口

        RenderOffscreen();   // 渲染图形

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

void InitOffscreen()
{
    // ================= Offscreen =================
    D3D11_TEXTURE2D_DESC td = {};
    td.Width            = g_width;
    td.Height           = g_height;
    td.MipLevels        = 1;
    td.ArraySize        = 1;
    td.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; 

    g_pd3dDevice->CreateTexture2D         (&td,      0, &g_offTex);
    if (!g_offTex) return;
    g_pd3dDevice->CreateRenderTargetView  (g_offTex, 0, &g_offRtv);
    g_pd3dDevice->CreateShaderResourceView(g_offTex, 0, &g_offSrv);

     
    // ================= Triangle =================  
    TVertex triangle[] =
    {
        { 0.0f,  0.5f, 0.0f, 1, 0, 0, 1},  // 顶部 红
        { 0.5f, -0.5f, 0.0f, 0, 1, 0, 1},  // 右下 绿
        {-0.5f, -0.5f, 0.0f, 0, 0, 1, 1},  // 左下 蓝
    };
      
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth         = sizeof(triangle);
    bd.BindFlags         = D3D11_BIND_VERTEX_BUFFER;  

    D3D11_SUBRESOURCE_DATA sdv = { triangle };  
    g_pd3dDevice->CreateBuffer(&bd, &sdv, &g_vb);
     
    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(XMMATRIX);
    cbd.Usage     = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 

    g_pd3dDevice->CreateBuffer(&cbd, nullptr, &g_vsConst);  
    if (!g_vsConst) return;

    const char* tsrc = R"( 
        cbuffer VSConstants : register(b0)
        {
            float4x4 vp;
        }; 

        struct VSInput
        {
            float3 pos : POSITION;
            float4 color : COLOR;
        };
        
        struct PSInput
        {
            float4 pos : SV_POSITION;
            float4 color : COLOR;
        };
        
        PSInput VSMain(VSInput input)
        {
            PSInput o;
            o.pos = mul(float4(input.pos, 1.0f), vp);
            o.color = input.color;
            return o;
        }
        
        float4 PSMain(PSInput input) : SV_TARGET
        {
            return input.color;
        }
     )";

    ID3DBlob* vBuffer = Compile(tsrc, "VSMain", "vs_5_0");
    ID3DBlob* pBuffer = Compile(tsrc, "PSMain", "ps_5_0");

    g_pd3dDevice->CreateVertexShader(vBuffer->GetBufferPointer(), vBuffer->GetBufferSize(), 0, &g_vs);
    g_pd3dDevice->CreatePixelShader (pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), 0, &g_ps);

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
         {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,   0, 0,D3D11_INPUT_PER_VERTEX_DATA,0 },
         {"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
    };

    g_pd3dDevice->CreateInputLayout(inputLayoutDesc, 2, vBuffer->GetBufferPointer(), vBuffer->GetBufferSize(), &g_layout);
    vBuffer->Release(); 
    pBuffer->Release();  
   
};

/// <summary>
/// Lesson101-OffScreenRendering
/// </summary>
void RenderOffscreen()
{
    if (!g_offTex || !g_offRtv || !g_offSrv) return;

    D3D11_TEXTURE2D_DESC desc;
    g_offTex->GetDesc(&desc);

    float c[4] = { 0.3f, 0.0f, 0.0f, 1.0f };
    g_pd3dDeviceContext->VSSetConstantBuffers (0, 1, &g_vsConst);
    g_pd3dDeviceContext->OMSetRenderTargets   (1, &g_offRtv, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_offRtv, c);

    //  用实际纹理尺寸，不再用 g_width/g_height
    D3D11_VIEWPORT vp = {};
    vp.Width    = (float)desc.Width;
    vp.Height   = (float)desc.Height;
    vp.MaxDepth = 1.0f;
    g_pd3dDeviceContext->RSSetViewports(1, &vp);

    D3D11_RECT rect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };
    g_pd3dDeviceContext->RSSetScissorRects(1, &rect);

    UINT stride = sizeof(TVertex), offset = 0;
    g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &g_vb, &stride, &offset);
    g_pd3dDeviceContext->IASetInputLayout(g_layout);
    g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    XMMATRIX viewProj = XMMatrixTranspose(g_viewProj);
    g_pd3dDeviceContext->UpdateSubresource(g_vsConst, 0, nullptr, &viewProj, 0, 0);

    g_pd3dDeviceContext->VSSetShader(g_vs, nullptr, 0);
    g_pd3dDeviceContext->PSSetShader(g_ps, nullptr, 0);
    g_pd3dDeviceContext->Draw(3, 0);
    g_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

}

/// <summary>
/// Lesson103-ImGuiViewport
/// </summary>
void RenderViewport()
{
    ViewportState state{};

    // ===== 默认初始化（防御式，避免脏值）=====
    state.delta = ImVec2(0, 0);
    state.wheel = 0.0f;
    state.mouse_local = ImVec2(-1, -1); //

    // ===== 1. 视口窗口 =====
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    ImGui::Begin("Lesson103-ImGuiViewport");

    ImVec2 size = ImGui::GetContentRegionAvail();
    state.viewport_size = size;

    // 每帧同步离屏纹理尺寸
    ResizeOffscreen((int)size.x, (int)size.y);

    // ===== 渲染图像 =====
    ImGui::Image(g_offSrv, size);

    state.viewport_min = ImGui::GetItemRectMin();
    state.viewport_max = ImGui::GetItemRectMax();

    // ===== 鼠标位置（安全获取）=====
    ImVec2 mouse = ImGui::GetMousePos();
    if (!ImGui::IsMousePosValid(&mouse))
    {
        mouse = ImVec2(0, 0);
    }
    state.mouse_pos = mouse;

    // ===== Hover 判断 =====
    bool hovered = ImGui::IsItemHovered();

    // ===== 局部坐标（仅在 hovered 时有效）=====
    if (hovered)
    {
        state.mouse_local.x = mouse.x - state.viewport_min.x;
        state.mouse_local.y = mouse.y - state.viewport_min.y;

        state.mouse_local.x = ImClamp(state.mouse_local.x, 0.0f, state.viewport_size.x);
        state.mouse_local.y = ImClamp(state.mouse_local.y, 0.0f, state.viewport_size.y);
    }

    // ===== 缩放（滚轮）=====
    if (hovered)
    {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            state.wheel = wheel;
        }
    }

    // ===== 平移（中键拖拽）=====
    if (hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        state.delta = ImGui::GetIO().MouseDelta;

        printf("Dragging: %.2f, %.2f\n", state.delta.x, state.delta.y);
    }

    // ===== 中键点击（可选调试）=====
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
    {
        printf("Middle Click\n");
    }

    ImGui::End();
    ImGui::PopStyleVar(2);

    // ===== 2. Debug 窗口 =====
    ImGui::Begin("Viewport Info");

    ImGui::Text("Viewport min   : %.1f, %.1f", state.viewport_min.x,  state.viewport_min.y);
    ImGui::Text("Viewport size  : %.1f, %.1f", state.viewport_size.x, state.viewport_size.y);
    ImGui::Text("Mouse pos      : %.1f, %.1f", state.mouse_pos.x,     state.mouse_pos.y);
    ImGui::Text("Mouse local    : %.1f, %.1f", state.mouse_local.x,   state.mouse_local.y);
    ImGui::Text("Delta          : %.1f, %.1f", state.delta.x,         state.delta.y);
    ImGui::Text("Wheel          : %.1f",       state.wheel);
    ImGui::Text("Camera zoom    : %.1f",       g_cam.zoom);
    ImGui::Text("Camera offset  : %.1f, %.1f", g_cam.offset.x,g_cam.offset.y);

    ImGui::End();

    UpdateViewProj(state);
}

void UpdateViewProj(ViewportState state)
{ 
    float w = state.viewport_size.x;
    float h = state.viewport_size.y;
    float aspect = (h > 0) ? (w / h) : 1.0f; 

    // ===== 1. 缩放 =====
    if (state.wheel != 0.0f)
    {
        float prevZoom = g_cam.zoom;
        g_cam.zoom *= powf(1.1f, state.wheel);
        g_cam.zoom = max(0.01f, g_cam.zoom);

        if (state.mouse_local.x >= 0)
        {
            // 鼠标归一化到 [0,1]（与单位无关）
            float nx = state.mouse_local.x / w;
            float ny = state.mouse_local.y / h;

            // ✅ 统一用 NDC 单位计算 halfW/halfH
            float halfW_prev = aspect / prevZoom;
            float halfH_prev = 1.0f / prevZoom;
            float worldX_before = g_cam.offset.x - halfW_prev + nx * (halfW_prev * 2.0f);
            float worldY_before = g_cam.offset.y + halfH_prev - ny * (halfH_prev * 2.0f);

            float halfW_now = aspect / g_cam.zoom;
            float halfH_now = 1.0f / g_cam.zoom;
            float worldX_after = g_cam.offset.x - halfW_now + nx * (halfW_now * 2.0f);
            float worldY_after = g_cam.offset.y + halfH_now - ny * (halfH_now * 2.0f);

            g_cam.offset.x += (worldX_before - worldX_after);
            g_cam.offset.y += (worldY_before - worldY_after);
        }
    }

    // ===== 2. 平移 =====
    if (state.delta.x != 0 || state.delta.y != 0)
    {
        // 每像素对应的 NDC 世界单位
        float worldPerPixelX = (aspect * 2.0f / g_cam.zoom) / w;
        float worldPerPixelY = (2.0f / g_cam.zoom) / h;

        g_cam.offset.x -= state.delta.x * worldPerPixelX;
        g_cam.offset.y += state.delta.y * worldPerPixelY;
    }

    // ===== 3. 投影矩阵（NDC单位，三角形 ±0.5 正好可见）=====
    float halfW = aspect / g_cam.zoom;
    float halfH = 1.0f / g_cam.zoom;

    g_viewProj = XMMatrixOrthographicOffCenterLH(
        g_cam.offset.x - halfW,  // left
        g_cam.offset.x + halfW,  // right
        g_cam.offset.y - halfH,  // bottom
        g_cam.offset.y + halfH,  // top
        0.0f, 1.0f
    );
}

// 动态重建纹理
void ResizeOffscreen(int w, int h)
{    
    if (w <= 0 || h <= 0) return;
    // 尺寸没变就跳过
    if (g_offTex)
    {
        D3D11_TEXTURE2D_DESC desc;
        g_offTex->GetDesc(&desc);
        if ((int)desc.Width == w && (int)desc.Height == h) return;
    }

    // 释放旧资源
    if (g_offSrv) { g_offSrv->Release(); g_offSrv = nullptr; }
    if (g_offRtv) { g_offRtv->Release(); g_offRtv = nullptr; }
    if (g_offTex) { g_offTex->Release(); g_offTex = nullptr; }

    // 重建
    D3D11_TEXTURE2D_DESC td = {};
    td.Width                = w;
    td.Height               = h;
    td.MipLevels            = 1;
    td.ArraySize            = 1;
    td.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count     = 1;
    td.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    g_pd3dDevice->CreateTexture2D         (&td, 0, &g_offTex);
    if (!g_offTex) return;
    g_pd3dDevice->CreateRenderTargetView  (g_offTex, 0, &g_offRtv);
    g_pd3dDevice->CreateShaderResourceView(g_offTex, 0, &g_offSrv);
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
