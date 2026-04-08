#pragma once
#include "pch.h"
#include "ErrorReporter.h"
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <format>
using  Microsoft::WRL::ComPtr;
using namespace DirectX;
namespace MiniCAD
{ 
    struct Vertex_P3_C4
    {
        XMFLOAT3 pos;
        XMFLOAT4 color;
    };

    struct Vertex_P3_N3_U2
    {
        XMFLOAT3 pos;
        XMFLOAT3 normal;
        XMFLOAT2 uv;
    };

    struct ShaderProgram
    {
        ComPtr<ID3D11VertexShader> vs;
        ComPtr<ID3D11PixelShader>  ps;
        ComPtr<ID3DBlob>           vsBlob;
    };

    struct PipelineState
    {
        ShaderProgram*           shader = nullptr;
        ID3D11InputLayout*       layout = nullptr;
        ID3D11DepthStencilState* depth = nullptr;
        D3D11_PRIMITIVE_TOPOLOGY topology;

        bool operator==(const PipelineState& other) const
        {
            return shader == other.shader && layout == other.layout &&  depth == other.depth &&  topology == other.topology;
        }
    };

    ShaderProgram CreateShader(ID3D11Device* device, const wchar_t* file);

    ComPtr<ID3D11InputLayout> CreateLayout(ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC* desc, UINT count, ID3DBlob* vsBlob);

    class LineShader // P3C4
    {
    public:
        void Initialize(ID3D11Device* device)
        {
            m_shader = CreateShader(device, L"Line.hlsl");

            D3D11_INPUT_ELEMENT_DESC desc[] =
            {
                {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
                {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
            };

            m_layout = CreateLayout(device, desc, 2, m_shader.vsBlob.Get());
        }

        PipelineState GetPipeline(ID3D11DepthStencilState* depth)
        {
            PipelineState pso;

            pso.shader    = &m_shader;
            pso.layout    = m_layout.Get();
            pso.topology  = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            pso.depth     = depth;

            return pso;
        }

    private:
        ShaderProgram m_shader;
        ComPtr<ID3D11InputLayout> m_layout;
    };
 
    class MeshShader // P3N3U2
    {
    public:
        void Initialize(ID3D11Device* device)
        {
            m_shader = CreateShader(device, L"Mesh.hlsl");

            D3D11_INPUT_ELEMENT_DESC desc[] =
            {
                {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
                {"NORMAL",  0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
                {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
            };

            m_layout = CreateLayout(device, desc, 3, m_shader.vsBlob.Get());
        }

        PipelineState GetPipeline(ID3D11DepthStencilState* depth)
        {
            PipelineState pso;
            pso.shader   = &m_shader;
            pso.layout   = m_layout.Get();
            pso.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            pso.depth    = depth;

            return pso;
        }

    private:
        ShaderProgram m_shader;
        ComPtr<ID3D11InputLayout> m_layout;
    };


}