#pragma once 
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Camera.h"
#include <unordered_set> 
namespace MiniCAD
{ 
    class Viewport
    {
    public:
        Viewport(Renderer& renderer, float width, float height);

        void Render(const RenderTarget& target, const std::vector<Vertex_P3_C4>& sceneVertices, const std::vector<Vertex_P3_C4>& overlayVertices);
        void Resize(float width, float height);

        Camera&        GetCamera();
        const  Camera& GetCamera() const;

        // 交互 
        void Pan (float dx, float dy);
        void Zoom(float delta, float mouseX, float mouseY);
    private:
        Camera       m_camera;
        Renderer&    m_renderer;  
        std::vector<Vertex_P3_C4> m_vertices;
        std::vector<Vertex_P3_C4> m_vertices1; 
    };
	
}
