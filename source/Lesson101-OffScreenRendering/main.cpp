#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

IDXGISwapChain*            gSwap    = nullptr;
ID3D11Device*              gDev     = nullptr;
ID3D11DeviceContext*       gCtx     = nullptr;

ID3D11RenderTargetView*    gBackRTV = nullptr;

// Offscreen(离屏渲染 核心三件套)
ID3D11Texture2D*           gOffTex  = nullptr;  // GPU 上真实存像素的“图片内存”
ID3D11RenderTargetView*    gOffRTV  = nullptr;  // 让 GPU “可以往这张纹理画东西”
ID3D11ShaderResourceView*  gOffSRV  = nullptr;  // 让 shader 可以读取这张纹理

// Triangle(三角形)
ID3D11Buffer*              gVB      = nullptr;
ID3D11VertexShader*        gVS      = nullptr;
ID3D11PixelShader*         gPS      = nullptr;
ID3D11InputLayout*         gLayout  = nullptr;

// Quad(屏幕)
ID3D11Buffer*              gQVB     = nullptr;
ID3D11VertexShader*        gQVS     = nullptr;
ID3D11PixelShader*         gQPS     = nullptr;
ID3D11InputLayout*         gQL      = nullptr;
ID3D11SamplerState*        gSamp    = nullptr;  // GPU“怎么从纹理里取颜色”

HWND gWnd;

int W = 800, H = 600;

// ================= Window =================
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(h, m, w, l);
}

void InitWindow()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"D3D";
    RegisterClass(&wc);

    gWnd = CreateWindow(wc.lpszClassName, L"D3D11 Offscreen", WS_OVERLAPPEDWINDOW, 100, 100, W, H, 0, 0, GetModuleHandle(0), 0);

    ShowWindow(gWnd, SW_SHOW);
}

// ================= Compile =================
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

// ================= Init D3D =================
void InitD3D()
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = W;
    sd.BufferDesc.Height = H;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = gWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &sd,
        &gSwap,
        &gDev,
        nullptr,
        &gCtx);

    // backbuffer
    ID3D11Texture2D* bb = nullptr;
    gSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb);
    gDev->CreateRenderTargetView(bb, 0, &gBackRTV);
    bb->Release();

    D3D11_VIEWPORT vp = { 0,0,(float)W,(float)H,0,1 };
    gCtx->RSSetViewports(1, &vp);

    // rasterizer
    D3D11_RASTERIZER_DESC rs = {};
    rs.FillMode = D3D11_FILL_SOLID;
    rs.CullMode = D3D11_CULL_NONE;

    ID3D11RasterizerState* r;
    gDev->CreateRasterizerState(&rs, &r);
    gCtx->RSSetState(r);

    // ================= Offscreen =================
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = W;
    td.Height = H;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

   
    gDev->CreateTexture2D(&td, 0, &gOffTex);
    gDev->CreateRenderTargetView(gOffTex, 0, &gOffRTV);
    gDev->CreateShaderResourceView(gOffTex, 0, &gOffSRV);

    // ================= Triangle =================
    struct V { float x, y, z; };

    V tri[] =
    {
        {0,0.5f,0},
        {0.5f,-0.5f,0},
        {-0.5f,-0.5f,0}
    };

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(tri);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sdv = { tri };
    gDev->CreateBuffer(&bd, &sdv, &gVB);

    const char* vs =
        "struct I{float3 p:POSITION;};"
        "struct O{float4 p:SV_POSITION;};"
        "O main(I i){O o;o.p=float4(i.p,1);return o;}";

    const char* ps =
        "float4 main():SV_Target{return float4(1,0.6,0.1,1);}";

    ID3DBlob* v = Compile(vs, "main", "vs_5_0");
    ID3DBlob* p = Compile(ps, "main", "ps_5_0");

    gDev->CreateVertexShader(v->GetBufferPointer(), v->GetBufferSize(), 0, &gVS);
    gDev->CreatePixelShader(p->GetBufferPointer(), p->GetBufferSize(), 0, &gPS);

    D3D11_INPUT_ELEMENT_DESC il[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
        D3D11_INPUT_PER_VERTEX_DATA,0}
    };

    gDev->CreateInputLayout(il, 1,
        v->GetBufferPointer(), v->GetBufferSize(), &gLayout);

    v->Release(); p->Release();

    // ================= Quad =================
    struct Q { float x, y, z, u, v; };

    Q quad[] =
    {
        {-1,-1,0,0,1},
        {-1, 1,0,0,0},
        { 1,-1,0,1,1},
        { 1, 1,0,1,0},
    };

    bd.ByteWidth = sizeof(quad);
    D3D11_SUBRESOURCE_DATA sdq = { quad };
    gDev->CreateBuffer(&bd, &sdq, &gQVB);

    const char* qvs =
        "struct I{float3 p:POSITION;float2 uv:TEXCOORD;};"
        "struct O{float4 p:SV_POSITION;float2 uv:TEXCOORD;};"
        "O main(I i){O o;o.p=float4(i.p,1);o.uv=i.uv;return o;}"; 

    // FIXED PS (关键修复点)
    const char* qps =
        "Texture2D    t0:register(t0);"
        "SamplerState s0:register(s0);"
        "struct PS_IN{float4 p:SV_POSITION;float2 uv:TEXCOORD;};"
        "float4 main(PS_IN i):SV_Target"
        "{return t0.Sample(s0,i.uv);}";


    v = Compile(qvs, "main", "vs_5_0");
    p = Compile(qps, "main", "ps_5_0");

    gDev->CreateVertexShader(v->GetBufferPointer(), v->GetBufferSize(), 0, &gQVS);
    gDev->CreatePixelShader(p->GetBufferPointer(), p->GetBufferSize(), 0, &gQPS);

    D3D11_INPUT_ELEMENT_DESC ql[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,
        D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,
        D3D11_INPUT_PER_VERTEX_DATA,0}
    };

    gDev->CreateInputLayout(ql, 2,
        v->GetBufferPointer(), v->GetBufferSize(), &gQL);

    v->Release(); p->Release();

    // sampler
    D3D11_SAMPLER_DESC s = {};
    s.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    s.AddressU = s.AddressV = s.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    gDev->CreateSamplerState(&s, &gSamp);
}

// ================= Render =================
void Render()
{
    float c[4] = { 0.3,0,0,1 };

    // ================= PASS 1 =================
    gCtx->OMSetRenderTargets(1, &gOffRTV, nullptr);
    gCtx->ClearRenderTargetView(gOffRTV, c);

    UINT stride = 12, offset = 0;

    gCtx->IASetVertexBuffers(0, 1, &gVB, &stride, &offset);
    gCtx->IASetInputLayout  (gLayout);
    gCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    gCtx->VSSetShader(gVS, nullptr, 0);
    gCtx->PSSetShader(gPS, nullptr, 0);

    gCtx->Draw(3, 0);

    // IMPORTANT CLEANUP
    ID3D11RenderTargetView* nullRT[1] = { nullptr };
    gCtx->OMSetRenderTargets(1, nullRT, nullptr);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    gCtx->PSSetShaderResources(0, 1, nullSRV);

    gCtx->Flush();

    // ================= PASS 2 =================
    gCtx->OMSetRenderTargets(1, &gBackRTV, nullptr);
    gCtx->ClearRenderTargetView(gBackRTV, c);

    stride = 20;

    gCtx->IASetVertexBuffers(0, 1, &gQVB, &stride, &offset);
    gCtx->IASetInputLayout(gQL);
    gCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    gCtx->VSSetShader(gQVS, nullptr, 0);
    gCtx->PSSetShader(gQPS, nullptr, 0);

    gCtx->PSSetShaderResources(0, 1, &gOffSRV);
    gCtx->PSSetSamplers(0, 1, &gSamp); 

    gCtx->Draw(4, 0);

    gSwap->Present(1, 0);
}

// ================= main =================
int main()
{
    InitWindow();
    InitD3D();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else Render();
    }
}
