#pragma once 
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Camera.h"
#include "Render/ViewState.h"
#include <unordered_set>  
#include "Cursor.h"
#include "Grid.h"
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

        void Render(const RenderTarget& target, const ViewState& viewState);
        void Resize(float width, float height);

        Camera&        GetCamera();
        const  Camera& GetCamera() const;

        // 交互 
        void Pan (float dx, float dy);
        void Zoom(float delta, float mouseX, float mouseY);
    private:
        SelectionGeometry BuildSelectionGeometry(const RenderTarget& target, const ViewState& viewState);
        void              AddDashedLine(std::vector<Vertex_P3_C4>& out, XMFLOAT3& a, XMFLOAT3& b, XMFLOAT4& color, float dashLen = 6.0f, float gapLen = 4.0f);
        GripGeometry      BuildGripGeometry(const RenderTarget& target, const ViewState& vs);
        SnapGeometry      BuildSnapGeometry(const RenderTarget& target, const ViewState& vs);
    private:
        Camera       m_camera;
        Renderer&    m_renderer;  
        Cursor       m_cursor;
        Grid         m_grid;
        std::vector<Vertex_P3_C4> m_vertices;
        std::vector<Vertex_P3_C4> m_vertices1;  
    };
	
}
