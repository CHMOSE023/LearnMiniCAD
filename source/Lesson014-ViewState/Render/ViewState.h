#pragma once 
#include "Render/D3D11/Shader.h"
#include <span>
namespace MiniCAD
{
    struct ViewState
    {
        // ===== Geometry =====
        std::span<const Vertex_P3_C4> Scene;
        std::span<const Vertex_P3_C4> Overlay; 

        // ===== Render flags =====
        bool ShowGrid  = true;
        bool ShowGizmo = true;
        bool ShowSnap  = true;
    };
}