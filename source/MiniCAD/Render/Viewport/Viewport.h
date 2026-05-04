#pragma once 
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Camera.h"
#include "Render/ViewState.h" 
#include "Cursor.h"
#include "Grid.h"
#include <d3d11.h>
#include <wrl/client.h>
namespace MiniCAD
{ 
    // 选择范围框
    struct SelectionGeometry
    {
        std::vector<Vertex_P3_C4> fill;
        std::vector<Vertex_P3_C4> border; 
    };

    struct GripGeometry
    {
        std::vector<Vertex_P3_C4> fill;
        std::vector<Vertex_P3_C4> border; 
    };

    struct SnapGeometry
    {
        std::vector<Vertex_P3_C4> fill;
        std::vector<Vertex_P3_C4> border;
    };
    class Viewport
    {
    public:
        Viewport(Renderer& renderer, float width, float height);

        void Render(const ViewState& viewState);
        void Resize(float width, float height);

        Camera&        GetCamera();
        const  Camera& GetCamera() const;

        // 交互 
        void Pan (float dx, float dy);
        void Zoom(float delta, float mouseX, float mouseY);
         
        float GetWidth() const { return m_width; };
        float GetHight() const { return m_height; };
    public:
        const RenderTarget& GetRenderTarget() const;
    private:
        void              AddDashedLine(std::vector<Vertex_P3_C4>& out, XMFLOAT3& a, XMFLOAT3& b, XMFLOAT4& color, float dashLen = 6.0f, float gapLen = 4.0f);
        SelectionGeometry BuildSelectionGeometry( const ViewState& viewState);
        GripGeometry      BuildGripGeometry(const ViewState& vs);
        SnapGeometry      BuildSnapGeometry(const ViewState& vs);
    private:
        float          m_width; 
        float          m_height;
        Camera         m_camera;
        Renderer&      m_renderer;  
        Cursor         m_cursor;
        Grid           m_grid;
        RenderTarget   m_renderTarget;
        D3D11_VIEWPORT m_d3dViewport;

        std::vector<Vertex_P3_C4> m_vertices;
        std::vector<Vertex_P3_C4> m_vertices1;  
    };
	
}
