#pragma once
#include "pch.h"
#include "RenderTarget.h"
#include "Shader.h"
#include <span>

namespace MiniCAD
{
    enum class PrimitiveType
    {
        Line,
        Triangle
    };

    class Renderer
    {
    public:
        Renderer(ID3D11Device* device, ID3D11DeviceContext* context);

        void BeginFrame(const RenderTarget& target);
        void EndFrame();

        void Submit(std::span<const Vertex_P3_C4> verts,
                    const XMMATRIX& viewProj,
                    PrimitiveType type,
                    bool depth = true,
                    bool blend = false);   //  新增

    private:
        void Initialize();

    private:
        ID3D11Device*        m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr;

        ComPtr<ID3D11Buffer> m_vb;
        ComPtr<ID3D11Buffer> m_cb;

        UINT m_maxVertices = 65536;

        // ===== states =====
        ComPtr<ID3D11DepthStencilState> m_depthEnabled;
        ComPtr<ID3D11DepthStencilState> m_depthDisabled;
        ComPtr<ID3D11DepthStencilState> m_depthReadOnly;  // 透明用

        ComPtr<ID3D11RasterizerState> m_rsNoCull;
        ComPtr<ID3D11BlendState>      m_blendAlpha;

        LineShader m_lineShader;
    };
}