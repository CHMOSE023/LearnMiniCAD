#include "Mesh.h"
#include <stdexcept>
#include "ErrorReporter.h"
namespace MiniCAD
{

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        : m_vertices(vertices), m_indices(indices)
    {
    }

    void Mesh::UploadToGPU(ID3D11Device* device)
    {
        if (!device) return; 

        // Vertex buffer
        D3D11_BUFFER_DESC vbDesc{};
        vbDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_vertices.size());
        vbDesc.Usage     = D3D11_USAGE_DEFAULT;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vbData{};
        vbData.pSysMem = m_vertices.data();

        if (FAILED(device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf())))
            ReportError("Failed to create vertex buffer"); 

        // Index buffer
        D3D11_BUFFER_DESC ibDesc{};
        ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * m_indices.size());
        ibDesc.Usage     = D3D11_USAGE_DEFAULT;
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA ibData{};
        ibData.pSysMem = m_indices.data();

        if (FAILED(device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.GetAddressOf())))
            ReportError("Failed to create index buffer"); 
    }
}