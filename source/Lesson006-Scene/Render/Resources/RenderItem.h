#pragma once
#include <memory>
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"

namespace MiniCAD
{ 
    class RenderItem
    {
    public:
        RenderItem(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity())
            : m_mesh(mesh)
            , m_material(material)
            , m_world(transform)
        {
        }

        std::shared_ptr<Mesh>     GetMesh()      const { return m_mesh; }
        std::shared_ptr<Material> GetMaterial()  const { return m_material; }
        DirectX::XMMATRIX         GetTransform() const { return m_world; }

        void SetTransform(const DirectX::XMMATRIX& t) { m_world = t; }

    private:
        std::shared_ptr<Mesh>     m_mesh;
        std::shared_ptr<Material> m_material;
        DirectX::XMMATRIX         m_world;
    };
}