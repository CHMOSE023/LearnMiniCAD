#pragma once
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>

namespace MiniCAD
{
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 uv;
    };
     
    class Mesh
    {
    public:
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        void UploadToGPU(ID3D11Device* device);

        ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
        ID3D11Buffer* GetIndexBuffer()  const { return m_indexBuffer.Get(); }
        size_t        GetIndexCount()   const { return m_indices.size(); }

    private:
        std::vector<Vertex>   m_vertices;
        std::vector<uint32_t> m_indices;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    };
}
