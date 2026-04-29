#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>   

IDXGISwapChain*            g_swap    = nullptr;
ID3D11Device*              g_dev     = nullptr;
ID3D11DeviceContext*       g_ctx     = nullptr;

ID3D11RenderTargetView*    g_backRTV = nullptr;

// Offscreen(离屏渲染 核心三件套)
ID3D11Texture2D*           g_offTex  = nullptr;  // GPU 上真实存像素的“图片内存”
ID3D11RenderTargetView*    g_offRtv  = nullptr;  // 让 GPU “可以往这张纹理画东西”
ID3D11ShaderResourceView*  g_offSRv  = nullptr;  // 让 shader 可以读取这张纹理 

// Triangle(三角形)
ID3D11Buffer*              g_vsConst = nullptr;
ID3D11Buffer*              g_vb      = nullptr;
ID3D11VertexShader*        g_vs      = nullptr;
ID3D11PixelShader*         g_ps      = nullptr;
ID3D11InputLayout*         g_layout  = nullptr;

// Quad(屏幕)
ID3D11Buffer*              g_qvb     = nullptr;
ID3D11VertexShader*        g_qvs     = nullptr;
ID3D11PixelShader*         g_qps     = nullptr;
ID3D11InputLayout*         g_ql      = nullptr;
ID3D11SamplerState*        g_samp    = nullptr;  // GPU“怎么从纹理里取颜色”

HWND g_wnd; 
int  g_wight = 800;
int  g_height = 600;

struct TVertex     { float x, y, z, r, g, b, a; };
struct QVertex     { float x, y, z, u, v; };
struct VSConstants { float vp[16]; };
  
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

    g_wnd = CreateWindow(wc.lpszClassName, L"D3D11 Offscreen", WS_OVERLAPPEDWINDOW, 100, 100, g_wight, g_height, 0, 0, GetModuleHandle(0), 0);

    ShowWindow(g_wnd, SW_SHOW);
}

// ================= Compile =================
ID3DBlob* Compile(const char* src, const char* entry, const char* target)
{
    ID3DBlob* blob = nullptr;
    ID3DBlob* err  = nullptr;
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
    sd.BufferCount          = 1;
    sd.BufferDesc.Width     = g_wight;
    sd.BufferDesc.Height    = g_height;
    sd.BufferDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow         = g_wnd;
    sd.SampleDesc.Count     = 1;
    sd.Windowed             = TRUE;

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &sd,
        &g_swap,
        &g_dev,
        nullptr,
        &g_ctx);

    // backbuffer
    ID3D11Texture2D* bb = nullptr;
    g_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb);
    g_dev->CreateRenderTargetView(bb, 0, &g_backRTV);
    bb->Release();

    // rasterizer
    D3D11_RASTERIZER_DESC rs = {};
    rs.FillMode              = D3D11_FILL_SOLID;
    rs.CullMode              = D3D11_CULL_NONE;

    ID3D11RasterizerState* r;
    g_dev->CreateRasterizerState(&rs, &r);
    g_ctx->RSSetState(r);

    // ================= Offscreen =================
    D3D11_TEXTURE2D_DESC td = {};
    td.Width                = g_wight;
    td.Height               = g_height;
    td.MipLevels            = 1;
    td.ArraySize            = 1;
    td.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count     = 1;
    td.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; 
   
    g_dev->CreateTexture2D         (&td,      0, &g_offTex);
    g_dev->CreateRenderTargetView  (g_offTex, 0, &g_offRtv);
    g_dev->CreateShaderResourceView(g_offTex, 0, &g_offSRv);

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
    g_dev->CreateBuffer(&bd, &sdv, &g_vb);

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth         = sizeof(VSConstants);
    cbd.Usage             = D3D11_USAGE_DEFAULT;
    cbd.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
    g_dev->CreateBuffer(&cbd, nullptr, &g_vsConst); 
     
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

    g_dev->CreateVertexShader(vBuffer->GetBufferPointer(), vBuffer->GetBufferSize(), 0, &g_vs);
    g_dev->CreatePixelShader (pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), 0, &g_ps);

    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    { 
         {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,   0, 0,D3D11_INPUT_PER_VERTEX_DATA,0 },
         {"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
    };

    g_dev->CreateInputLayout(inputLayoutDesc, 2, vBuffer->GetBufferPointer(), vBuffer->GetBufferSize(), &g_layout);

    vBuffer->Release(); pBuffer->Release();
      
    // ================= Quad =================  
    QVertex quad[] =
    {
        {-1,-1,0,0,1},
        {-1, 1,0,0,0},
        { 1,-1,0,1,1},
        { 1, 1,0,1,0},
    };

    bd.ByteWidth = sizeof(quad);
    D3D11_SUBRESOURCE_DATA sdq = { quad };
    g_dev->CreateBuffer(&bd, &sdq, &g_qvb);

    const char* qsrc = R"(
        struct VSInput
        {
            float3 pos : POSITION;
            float2 uv  : TEXCOORD;
        };
    
        struct PSInput
        {
            float4 pos : SV_POSITION;
            float2 uv  : TEXCOORD;
        };
    
        PSInput VSMain(VSInput input)
        {
            PSInput o;
            o.pos = float4(input.pos, 1.0f);
            o.uv  = input.uv;
            return o;
        }
    
        Texture2D    t0 : register(t0);
        SamplerState s0 : register(s0);
    
        float4 PSMain(PSInput input) : SV_TARGET
        {
            return t0.Sample(s0, input.uv);
        }
    )";


    vBuffer = Compile(qsrc, "VSMain", "vs_5_0");
    pBuffer = Compile(qsrc, "PSMain", "ps_5_0");

    g_dev->CreateVertexShader(vBuffer->GetBufferPointer(), vBuffer->GetBufferSize(), 0, &g_qvs);
    g_dev->CreatePixelShader (pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), 0, &g_qps);

    D3D11_INPUT_ELEMENT_DESC ql[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,  0, D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,   0, 12, D3D11_INPUT_PER_VERTEX_DATA,0}
    };

    g_dev->CreateInputLayout(ql, 2, vBuffer->GetBufferPointer(), vBuffer->GetBufferSize(), &g_ql);

    vBuffer->Release(); pBuffer->Release();

    // sampler
    D3D11_SAMPLER_DESC s = {};
    s.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    s.AddressU = s.AddressV = s.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; 

    g_dev->CreateSamplerState(&s, &g_samp);
}

// ================= Render =================
void Render()
{
    float c[4] = { 0 ,0,0,1 };


    // ================= PASS 1 =================   

    D3D11_VIEWPORT vp = { 0,0,(float)g_wight,(float)g_height,0,1 };
    g_ctx->RSSetViewports(1, &vp);

    g_ctx->OMSetRenderTargets   (1, &g_offRtv, nullptr);
    g_ctx->ClearRenderTargetView(g_offRtv, c);

    UINT stride = sizeof(TVertex); 
    UINT offset = 0;

    g_ctx->IASetVertexBuffers(0, 1, &g_vb, &stride, &offset);
    g_ctx->IASetInputLayout  (g_layout);
    g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      
    // 单位矩阵
    VSConstants data = { 1,0,0,0,
                         0,1,0,0,
                         0,0,1,0,
                         0,0,0,1 };

    g_ctx->UpdateSubresource(g_vsConst, 0, nullptr, &data, 0, 0);
    g_ctx->VSSetConstantBuffers(0, 1, &g_vsConst);
    g_ctx->VSSetShader(g_vs, nullptr, 0);
    g_ctx->PSSetShader(g_ps, nullptr, 0);
    g_ctx->Draw(3, 0);

    // IMPORTANT CLEANUP
    ID3D11RenderTargetView* nullRT[1] = { nullptr };
    g_ctx->OMSetRenderTargets(1, nullRT, nullptr);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    g_ctx->PSSetShaderResources(0, 1, nullSRV);

    g_ctx->Flush();

    // ================= PASS 2 =================
    g_ctx->OMSetRenderTargets(1, &g_backRTV, nullptr);
    g_ctx->ClearRenderTargetView(g_backRTV, c);

    stride = sizeof(QVertex);

    g_ctx->IASetVertexBuffers(0, 1, &g_qvb, &stride, &offset);
    g_ctx->IASetInputLayout(g_ql);
    g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    g_ctx->VSSetShader(g_qvs, nullptr, 0);
    g_ctx->PSSetShader(g_qps, nullptr, 0);

    g_ctx->PSSetShaderResources(0, 1, &g_offSRv);
    g_ctx->PSSetSamplers(0, 1, &g_samp); 

    g_ctx->Draw(4, 0);

    g_swap->Present(1, 0);
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

