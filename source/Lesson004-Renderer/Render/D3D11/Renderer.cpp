#include "Renderer.h" 
#include <d3dcompiler.h>
#include <format>
#include "ErrorReporter.h"
using Microsoft::WRL::ComPtr;

namespace MiniCAD
{
    Renderer::Renderer(ID3D11Device* device, ID3D11DeviceContext* context) 
            : m_device(device)
            , m_context(context)
    {
        Initialize();
        m_lineShader.Initialize(m_device);
        m_gripShader.Initialize(m_device);
    } 

    void Renderer::Initialize()
    { 
        // ==== 动态顶点缓冲（关键优化）====
        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.Usage          = D3D11_USAGE_DYNAMIC;
        vbDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        vbDesc.ByteWidth      = sizeof(Vertex_P3_C4) * m_maxVertices;
		m_device->CreateBuffer(&vbDesc, nullptr, m_lineVB.GetAddressOf());  // Line 专用

        vbDesc.ByteWidth = sizeof(Vertex_P3) * m_maxVertices;
		m_device->CreateBuffer(&vbDesc, nullptr, m_gripVB.GetAddressOf());  // Grip 专用
         
        // ==== 常量缓冲 ====
        D3D11_BUFFER_DESC cbDesc = {}; 
        cbDesc.Usage     = D3D11_USAGE_DEFAULT;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 

        // 常量 viewProj  
        cbDesc.ByteWidth = sizeof(XMMATRIX);
        m_device->CreateBuffer(&cbDesc, nullptr, m_viewProjCB.GetAddressOf());

        // 常量 color 
        cbDesc.ByteWidth = sizeof(XMFLOAT4); 
        m_device->CreateBuffer(&cbDesc, nullptr, m_colorCB.GetAddressOf());

        // ==== 深度状态 ====
        // 深度测试启用 
        D3D11_DEPTH_STENCIL_DESC depthEnableDesc = {};

        depthEnableDesc.DepthEnable    = TRUE;
        depthEnableDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthEnableDesc.DepthFunc      = D3D11_COMPARISON_LESS;

        m_device->CreateDepthStencilState(&depthEnableDesc, m_depthStateEnabled.GetAddressOf());

        // 深度测试禁用 
        D3D11_DEPTH_STENCIL_DESC depthDisableDesc = {};
        depthDisableDesc.DepthEnable    = FALSE;
        depthDisableDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        m_device->CreateDepthStencilState(&depthDisableDesc, m_depthStateDisabled.GetAddressOf());

        // ==== 混合状态（透明）====
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        m_device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
    }

    void Renderer::Begin(const RenderTarget& target)
    {
        target.Bind(m_context);

		float clear[4] = { 0.1f, 0.1f, 0.15f, 1 }; // 窗口背景色
        target.Clear(m_context, clear);
           
        m_lineBuffer.clear();
        m_gripBuffer.clear();
    }

    void Renderer::DrawLine(const std::vector<Vertex_P3_C4>& verts, const XMMATRIX& vp, bool depth)
    {
        XMMATRIX mat = XMMatrixTranspose(vp); 

        m_context->UpdateSubresource   (m_viewProjCB.Get(), 0, nullptr, &mat, 0, 0);
         
        auto pso = m_lineShader.GetPipeline();

        BindPipeline(pso);

        SetDepthEnabled(depth);

        m_lineBuffer.insert(m_lineBuffer.end(), verts.begin(), verts.end());
    }

    void Renderer::DrawGrip(const std::vector<Vertex_P3>& verts,  XMFLOAT4 color, const XMMATRIX& vp, bool depth)
    {   
        BindPipeline(m_gripShader.GetPipeline());

        XMMATRIX mat = XMMatrixTranspose(vp);
        m_context->UpdateSubresource(m_viewProjCB.Get(), 0, nullptr, &mat, 0, 0);
        m_context->UpdateSubresource(m_colorCB.Get(), 0, nullptr, &color, 0, 0);
    
        SetDepthEnabled(depth);

        m_gripBuffer.insert(m_gripBuffer.end(), verts.begin(), verts.end());
    }

    void Renderer::BindPipeline(const PipelineState& pso)
    {
        // if (m_currentPSO == pso) return; 

        if (m_currentPSO.shader != nullptr) // 避免初始状态下的无效 Flush
        { 
            if (m_currentPSO.topology == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) // 切换前如果是 Line PSO，先提交 Line 缓冲区
                FlushLine();
            else
                FlushGrip();
        } 

        m_currentPSO = pso;  
        m_context->IASetInputLayout      (pso.layout);
        m_context->VSSetShader           (pso.shader->vs.Get(), nullptr, 0);
        m_context->PSSetShader           (pso.shader->ps.Get(), nullptr, 0); 
        m_context->IASetPrimitiveTopology(pso.topology);
    }

    void Renderer::FlushLine()
    {
        if (m_lineBuffer.empty()) return;   

        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_lineVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, m_lineBuffer.data(), sizeof(Vertex_P3_C4) * m_lineBuffer.size());
        m_context->Unmap(m_lineVB.Get(), 0);

        UINT stride = sizeof(Vertex_P3_C4);
        UINT offset = 0;

        m_context->IASetVertexBuffers(0, 1, m_lineVB.GetAddressOf(), &stride, &offset);     // 设置顶点
        m_context->VSSetConstantBuffers(0, 1, m_viewProjCB.GetAddressOf());                 // 设置VP常量 投影矩阵
         
        m_context->Draw((UINT)m_lineBuffer.size(), 0);

        m_lineBuffer.clear();
    }

    void Renderer::FlushGrip()
    {
        if (m_gripBuffer.empty()) return; 
        
        float blendFactor[4] = { 0,0,0,0 };
        m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);         // 开启透明混合
         
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_gripVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

        memcpy(mapped.pData, m_gripBuffer.data(), sizeof(Vertex_P3) * m_gripBuffer.size());
        m_context->Unmap(m_gripVB.Get(), 0);

        UINT stride = sizeof(Vertex_P3);
        UINT offset = 0;
        m_context->IASetVertexBuffers(0, 1, m_gripVB.GetAddressOf(), &stride, &offset);    // 设置顶点

        m_context->VSSetConstantBuffers(0, 1, m_viewProjCB.GetAddressOf());                // 设置VP常量 投影矩阵
        m_context->PSSetConstantBuffers(1, 1, m_colorCB.GetAddressOf());                   // 设置PS常量 颜色
        m_context->Draw((UINT)m_gripBuffer.size(), 0);

        m_context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);                     // 关闭透明混合，恢复默认
        m_gripBuffer.clear();
    }

    void Renderer::End()
    {
        FlushLine();    
        FlushGrip();   
    }

    void Renderer::SetDepthEnabled(bool enabled)
    {
        if (enabled)
            m_context->OMSetDepthStencilState(m_depthStateEnabled.Get(), 0);
        else
            m_context->OMSetDepthStencilState(m_depthStateDisabled.Get(), 0);
    }
}
