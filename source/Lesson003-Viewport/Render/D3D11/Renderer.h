#pragma once 
#include "pch.h"
#include "RenderTarget.h" 

using namespace DirectX;

namespace MiniCAD
{
    struct LineVertex
    {
        XMFLOAT3 pos;
        XMFLOAT4 color;
    };  

    class Renderer
    {
    public:
        Renderer(ID3D11Device* device, ID3D11DeviceContext* context);
        void Begin(const RenderTarget& target, const XMMATRIX& mvp); 
        void Submit(const LineVertex* verts, UINT count);
        void End();
        void SetDepthEnabled(bool enabled);
    private:
        void Initialize();
        void Flush(D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    private:
        ID3D11Device*        m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        // Pipeline
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_layout;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStateEnabled;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStateDisabled;

        // Buffers
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cb;

        UINT m_maxVertices = 65536;
        std::vector<LineVertex> m_cpuBuffer; 

        float m_screenW = 0.f, m_screenH = 0.f;
    };

}
