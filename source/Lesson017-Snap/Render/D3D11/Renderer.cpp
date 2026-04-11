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
		// 初始化 shader 和 pipeline state
        m_lineShader.Initialize(m_device);
        m_meshShader.Initialize(m_device); 
    }
     
    
    void Renderer::BindPipeline(const PipelineState& pso)
    {
        if (m_currentPSO.shader == pso.shader &&
            m_currentPSO.layout == pso.layout &&
            m_currentPSO.depth == pso.depth &&
            m_currentPSO.topology == pso.topology)

            return;

        Flush(); // 关键

        m_currentPSO = pso;

        m_context->IASetInputLayout(pso.layout);
        m_context->VSSetShader(pso.shader->vs.Get(), nullptr, 0);
        m_context->PSSetShader(pso.shader->ps.Get(), nullptr, 0);
        m_context->OMSetDepthStencilState(pso.depth, 0);
        m_context->IASetPrimitiveTopology(pso.topology);
    }

    void Renderer::Initialize()
    {   
        // ==== 动态顶点缓冲（关键优化）====
        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.ByteWidth = sizeof(Vertex_P3_C4) * m_maxVertices;
        vbDesc.Usage = D3D11_USAGE_DYNAMIC;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        m_device->CreateBuffer(&vbDesc, nullptr, m_vb.GetAddressOf());

        // ==== 常量缓冲 ====
        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(XMMATRIX);
        cbDesc.Usage = D3D11_USAGE_DEFAULT;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        m_device->CreateBuffer(&cbDesc, nullptr, m_cb.GetAddressOf());

        // ==== 深度状态 ====
        // 深度测试启用（用于一般绘制）
        D3D11_DEPTH_STENCIL_DESC depthEnableDesc = {};
        depthEnableDesc.DepthEnable = TRUE;
        depthEnableDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthEnableDesc.DepthFunc = D3D11_COMPARISON_LESS;
        m_device->CreateDepthStencilState(&depthEnableDesc, m_depthEnabled.GetAddressOf());

        // 深度测试禁用（用于光标，确保总是显示）
        D3D11_DEPTH_STENCIL_DESC depthDisableDesc = {};
        depthDisableDesc.DepthEnable = FALSE;
        depthDisableDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        m_device->CreateDepthStencilState(&depthDisableDesc, m_depthDisabled.GetAddressOf());
    }

    void Renderer::BeginFrame(const RenderTarget& target)
    {
        target.Bind(m_context); 

        float clear[4] = { 0.1f, 0.1f, 0.15f, 1 };
        target.Clear(m_context, clear);  


        // m_cpuBuffer.clear(); 
    }

    void Renderer::BeginPass(const RenderPassDesc& pass)
    {
        Flush(); // 切 pass 必须 flush
 
        XMMATRIX mat = XMMatrixTranspose(pass.ViewProj);
        m_context->UpdateSubresource(m_cb.Get(), 0, nullptr, &mat, 0, 0);
        m_context->VSSetConstantBuffers(0, 1, m_cb.GetAddressOf());
        m_context->OMSetDepthStencilState(pass.Depth ? m_depthEnabled.Get() : m_depthDisabled.Get(), 0); 
    }

    void Renderer::EndPass()
    {

    };


    void Renderer::SubmitLine(const Vertex_P3_C4* verts, UINT count)
    {
        m_cpuBuffer.insert(m_cpuBuffer.end(), verts, verts + count);
    }

    void Renderer::DrawLine(const std::vector<Vertex_P3_C4>& verts, bool depth)
    {
        auto pso = m_lineShader.GetPipeline(depth ? m_depthEnabled.Get() : m_depthDisabled.Get());

        BindPipeline(pso);

        m_cpuBuffer.insert(m_cpuBuffer.end(), verts.begin(), verts.end());
    }

    void Renderer::DrawMesh(const Mesh& mesh)
    {
        auto pso = m_meshShader.GetPipeline(m_depthEnabled.Get());
        BindPipeline(pso);

        Flush(); // mesh 不走 batch

        UINT stride = sizeof(Vertex_P3_N3_U2);
        UINT offset = 0;

        ID3D11Buffer* vb = mesh.GetVertexBuffer();
        m_context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        m_context->IASetIndexBuffer(mesh.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
        m_context->DrawIndexed(mesh.GetIndexCount(), 0, 0);
    }
     
    //  提交 GPU
    void Renderer::Flush()
    {
        if (m_cpuBuffer.empty()) return;

        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

        memcpy(mapped.pData, m_cpuBuffer.data(), sizeof(Vertex_P3_C4) * m_cpuBuffer.size());

        m_context->Unmap(m_vb.Get(), 0);

        UINT stride = sizeof(Vertex_P3_C4);
        UINT offset = 0;

        m_context->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &stride, &offset);

        // 不要再设 topology
        m_context->Draw((UINT)m_cpuBuffer.size(), 0);

		m_cpuBuffer.clear();  // 清空 CPU buffer，等待下一批数据
    }
     

    void Renderer::EndFrame()
    {
        Flush();
    }
}
