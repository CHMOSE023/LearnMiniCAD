#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Render/D3D11/Shader.h"  // Vertex_P3_C4

namespace MiniCAD
{
    class Camera;

    class Grid
    {
    public:
        void Build(const Camera* camera, float screenWidth, float screenHeight);
        const std::vector<Vertex_P3_C4>& GetVerts() const { return m_verts; }

    private:
        std::vector<Vertex_P3_C4> m_verts;
    };
}