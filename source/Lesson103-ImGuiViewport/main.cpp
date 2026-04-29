#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h> 
#include <cstdio>
#include <d3dcompiler.h> 
#include <algorithm>

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
                                
// Triangle(三角形)        
ID3D11Buffer*                   g_vsConst = nullptr;  // 位置
ID3D11Buffer*                   g_vb      = nullptr;  // 顶点
ID3D11VertexShader*             g_vs      = nullptr;
ID3D11PixelShader*              g_ps      = nullptr;
ID3D11InputLayout*              g_layout  = nullptr;

int  g_width  = 1800;
int  g_height = 1600;

void InitOffscreen();
void RenderOffscreen();
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


struct TVertex     { float x, y, z, r, g, b, a; }; 
struct VSConstants { float vp[16]; };

struct ViewportState
{
    // --- 图像信息 ---
    ImVec2 image_min;   // 屏幕坐标：左上角
    ImVec2 image_size;  // 显示尺寸（窗口中）

    // --- 输入 ---
    ImVec2 mouse_pos;   // 鼠标屏幕坐标
    ImVec2 mouse_local; // 相对图像坐标
    ImVec2 uv;          // 0~1 归一化坐标

    // --- 相机（核心）---
    ImVec2 offset = ImVec2(0, 0); // 平移（世界坐标）
    float  scale = 1.0f;          // 缩放

    // --- 纹理 ---
    ImVec2 texture_size = ImVec2(1024, 1024); // 实际纹理尺寸

    // --- 推导结果 ---
    ImVec2 world_pos; // 鼠标在“图像世界中的位置”
};

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

        // Lesson024 -> 2.离屏渲染三角形
        RenderOffscreen();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        
        ViewportState state;
        state.texture_size = ImVec2((float)g_width, (float)g_height);
         
        // 1.显示图像, 
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            ImGui::Begin("Lesson103-ImGuiViewport");

            ImVec2 size = ImGui::GetContentRegionAvail();
            state.image_size = size;

            static bool initialized = false;
            if (!initialized)
            {
                state.offset.x = (state.texture_size.x - size.x) * 0.5f;
                state.offset.y = (state.texture_size.y - size.y) * 0.5f;
                initialized = true;
            }
             
            // ===== 3. 绘制图像 =====
            ImGui::Image(g_offSrv, size);

            // ===== 4. 鼠标 & 坐标 =====
            state.image_min    = ImGui::GetItemRectMin();
            state.mouse_pos    = ImGui::GetMousePos();

            state.mouse_local.x = state.mouse_pos.x - state.image_min.x;
            state.mouse_local.y = state.mouse_pos.y - state.image_min.y;

            // UV
            state.uv.x = state.mouse_local.x / state.image_size.x;
            state.uv.y = state.mouse_local.y / state.image_size.y;

            state.uv.x = ImClamp(state.uv.x, 0.0f, 1.0f);
            state.uv.y = ImClamp(state.uv.y, 0.0f, 1.0f);

            // 世界坐标
            state.world_pos.x = state.mouse_local.x / state.scale + state.offset.x;
            state.world_pos.y = state.mouse_local.y / state.scale + state.offset.y;

            // =====   缩放（滚轮）=====
            if (ImGui::IsItemHovered())
            {
                float wheel = ImGui::GetIO().MouseWheel;

                if (wheel != 0.0f)
                {
                    // 缩放前鼠标世界坐标
                    ImVec2 mouse_before = state.world_pos;

                    // 修改缩放
                    state.scale += wheel * 0.1f;
                    state.scale = ImClamp(state.scale, 0.1f, 10.0f);

                    // 缩放后重新计算
                    ImVec2 mouse_after;
                    mouse_after.x = state.mouse_local.x / state.scale + state.offset.x;
                    mouse_after.y = state.mouse_local.y / state.scale + state.offset.y;

                    // 调整offset（关键）
                    state.offset.x += (mouse_before.x - mouse_after.x);
                    state.offset.y += (mouse_before.y - mouse_after.y);
                }
            }

            // ===== 6. 平移（右键拖拽）=====
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                ImVec2 delta = ImGui::GetIO().MouseDelta;

                state.offset.x -= delta.x / state.scale;
                state.offset.y -= delta.y / state.scale;
            }

            ImGui::End();
            ImGui::PopStyleVar(2);
        }
         
        {
            // ===== 7. 调试信息窗口 =====
            ImGui::Begin("Image Info");

            ImGui::Text("Mouse Local : %.1f, %.1f", state.mouse_local.x, state.mouse_local.y);
            ImGui::Text("UV          : %.3f, %.3f", state.uv.x, state.uv.y);
            ImGui::Text("World       : %.1f, %.1f", state.world_pos.x, state.world_pos.y);
            ImGui::Text("Offset      : %.1f, %.1f", state.offset.x, state.offset.y);
            ImGui::Text("Scale       : %.2f", state.scale);

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
    g_pd3dDevice->CreateRenderTargetView  (g_offTex, 0, &g_offRtv);
    g_pd3dDevice->CreateShaderResourceView(g_offTex, 0, &g_offSrv);

     
    // ================= Triangle =================  
    TVertex triangle[] =
    {
        { 0.0f,  0.5f, 0.0f, 1, 0, 0, 1},  // 顶部 红
        { 0.5f, -0.5f, 0.0f, 0, 1, 0, 1},  // 右下 绿
        {-0.5f, -0.5f, 0.0f, 0, 0, 1, 1},  // 左下 蓝
    };

    // 单位矩阵
    VSConstants mat =  { 1,0,0,0,
                         0,1,0,0,
                         0,0,1,0,
                         0,0,0,1 }; 

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth         = sizeof(triangle);
    bd.BindFlags         = D3D11_BIND_VERTEX_BUFFER;  

    D3D11_SUBRESOURCE_DATA sdv = { triangle };  
    g_pd3dDevice->CreateBuffer(&bd, &sdv, &g_vb);


    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(VSConstants);
    cbd.Usage     = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 

    g_pd3dDevice->CreateBuffer(&cbd, nullptr, &g_vsConst);  

    g_pd3dDeviceContext->UpdateSubresource(g_vsConst, 0, nullptr, &mat, 0, 0);

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

void RenderOffscreen()
{
    float c[4] = { 0.3f, 0.0f, 0.0f, 1.0f };

  
    g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &g_vsConst);


    // ================= 绑定离屏 RT =================
    g_pd3dDeviceContext->OMSetRenderTargets   (1, &g_offRtv, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_offRtv, c);

    // ================= 关键：设置 Viewport =================
    D3D11_TEXTURE2D_DESC desc;
    g_offTex->GetDesc(&desc);

    D3D11_VIEWPORT vp;
    vp.TopLeftX  = 0.0f;
    vp.TopLeftY  = 0.0f;
    vp.Width     = g_width;
    vp.Height    = g_height;
    vp.MinDepth  = 0.0f;
    vp.MaxDepth  = 1.0f;

    g_pd3dDeviceContext->RSSetViewports(1, &vp);

    // （可选但推荐）Scissor
    D3D11_RECT rect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };
    g_pd3dDeviceContext->RSSetScissorRects(1, &rect);

    // ================= 输入装配 =================
    UINT stride = sizeof(TVertex);
    UINT offset = 0;

    g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &g_vb, &stride, &offset);
    g_pd3dDeviceContext->IASetInputLayout(g_layout);
    g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ================= shader =================
    g_pd3dDeviceContext->VSSetShader(g_vs, nullptr, 0);
    g_pd3dDeviceContext->PSSetShader(g_ps, nullptr, 0);

    // ================= draw =================
    g_pd3dDeviceContext->Draw(3, 0);

    g_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

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
