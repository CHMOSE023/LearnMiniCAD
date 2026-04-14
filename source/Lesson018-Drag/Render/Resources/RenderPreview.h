
#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Render/D3D11/Renderer.h"
namespace MiniCAD
{  
    struct RenderPreview
    {
        std::vector<Vertex_P3_C4> vertices; // 线段列表
    };
}
