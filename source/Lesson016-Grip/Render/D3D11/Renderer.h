#pragma once 
#include "pch.h"
#include "RenderTarget.h" 
#include "Shader.h"
#include <Render/Resources/Mesh.h>
#include "RenderPass.h"

using namespace DirectX;
using namespace Microsoft::WRL;
namespace MiniCAD
{ 

    class Renderer
    {
    public:
        Renderer(ID3D11Device* device, ID3D11DeviceContext* context);

        void BeginFrame (const RenderTarget& target);
        void EndFrame();

        void BeginPass  (const RenderPassDesc& pass);
        void EndPass();

        void DrawLine  (const std::vector<Vertex_P3_C4>& verts, bool depth = true);
        void DrawMesh  (const Mesh& mesh); 

        ID3D11DepthStencilState* GetDepthEnabled()  const { return m_depthEnabled.Get(); }
        ID3D11DepthStencilState* GetDepthDisabled() const { return m_depthDisabled.Get(); }

    private:
        void Initialize();
        void Flush();
        void BindPipeline(const PipelineState& pso);
        void SubmitLine(const Vertex_P3_C4* verts, UINT count);
    private:
        ID3D11Device*        m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr; 
        
		// Pipeline
        ComPtr<ID3D11DepthStencilState> m_depthEnabled;
        ComPtr<ID3D11DepthStencilState> m_depthDisabled;

        // Buffers
        ComPtr<ID3D11Buffer> m_vb;
        ComPtr<ID3D11Buffer> m_cb;

        UINT m_maxVertices = 65536;

        std::vector<Vertex_P3_C4> m_cpuBuffer;

        float m_screenW = 0.f, m_screenH = 0.f;

        LineShader       m_lineShader;
        MeshShader       m_meshShader;
        PipelineState    m_currentPSO = {};

    };

}
