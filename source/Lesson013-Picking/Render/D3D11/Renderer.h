#pragma once 
#include "pch.h"
#include "RenderTarget.h" 
#include "Shader.h"

using namespace DirectX;

namespace MiniCAD
{  
    class Renderer
    {
    public:
        Renderer(ID3D11Device* device, ID3D11DeviceContext* context);
        void Begin   (const RenderTarget& target);
        void DrawLine(const std::vector<Vertex_P3_C4>& verts, const XMMATRIX& vp, bool depth = true);
        void DrawGrip(const std::vector<Vertex_P3>& verts, XMFLOAT4 color, const XMMATRIX& vp, bool depth = true);
        void End(); 
    private:
        void SetDepthEnabled(bool enabled);
        void FlushLine();
        void FlushGrip();
        void Initialize();
        void BindPipeline(const PipelineState& pso);

    private:
        ID3D11Device*        m_device = nullptr;
        ID3D11DeviceContext* m_context = nullptr; 
    private:
        // Pipeline 
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStateEnabled;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStateDisabled;
        Microsoft::WRL::ComPtr<ID3D11BlendState>        m_blendState;

        ULONG64 m_maxVertices = 65536;  // 每个批次最大点数量

        // shader 相关状态
        PipelineState    m_currentPSO = {};

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_viewProjCB; // 视图投影矩阵常量缓冲区 
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_colorCB;    // 颜色常量缓冲区（Grip 专用）

        // Line
		LineShader                           m_lineShader;  // 用于绘制一般线条
        std::vector<Vertex_P3_C4>            m_lineBuffer;  // Line 专用
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_lineVB;      // Line 顶点 

        // Grip 
		GripShader                           m_gripShader;  // 用于绘制光标，始终显示在最前面
        std::vector<Vertex_P3>               m_gripBuffer;  // Grip 专用
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_gripVB;      // Grip 
       

        float m_screenW = 0.f;
        float m_screenH = 0.f;


    };

}
