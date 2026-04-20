#include "Viewport.h"
#include <DirectXMath.h>
#include "Render/D3D11/Renderer.h" 
#include "FractalGeometryGenerator.hpp"
#include "Camera.h"
using namespace DirectX;

namespace MiniCAD
{ 
    inline static XMFLOAT4 GetColor(int index, int maxIndex)
    {
        float t = (float)index / (float)maxIndex;
          
        return {
            0.65f,
            0.2f + 0.8f * (1.0f - t),
            0.3f + 0.7f * t,
            1.0f
        };
    }

    inline static void ConvertToVertices(const std::vector<MiniCAD::Line>& lines, std::vector<Vertex_P3_C4>& out)
    {
        int maxIndex = (int)lines.size();

        for (int i = 0; i < maxIndex; i++)
        {
            const auto& l = lines[i];
            XMFLOAT4 c = GetColor(i, maxIndex);
            out.push_back({ l.Start, c });
            out.push_back({ l.End,   c });
        }
    }

    Viewport::Viewport(Renderer& renderer,float width, float height)
        : m_renderer(renderer) 
        , m_camera(width, height)
    {
        
        MiniCAD::FractalGeometryGenerator gen;
         
        // 谢尔宾斯基地毯
        std::vector<MiniCAD::Line> lines;
        MiniCAD::FractalGeometryGenerator::Rect r;
        r.min = { -7,0,0 };
        r.max = { -1,6,0 };

        gen.GenerateSierpinskiCarpet(r, 4, lines);

        float n = 6;
        // 递归分型
        std::vector<MiniCAD::Line> lines1;
        MiniCAD::Line L0({ 0,0,0 }, { n,0,0 });
        MiniCAD::Line L1({ n,0,0 }, { n,n,0 });
        MiniCAD::Line L2({ n,n,0 }, { 0,n,0 });
        MiniCAD::Line L3({ 0,n,0 }, { 0,0,0 });
        gen.GenerateRecurseQuad(L0, L1, L2, L3, 50, lines1);

        // 转 GPU
        ConvertToVertices(lines, m_vertices);
        ConvertToVertices(lines1, m_vertices1);

    }
     
    void Viewport::Render(const RenderTarget& target, const std::vector<Vertex_P3_C4>& sceneVertices)
    {  
        XMMATRIX vp = m_camera.GetViewProj();
         
        m_renderer.Begin(target); 
         
        m_renderer.DrawLine(sceneVertices, vp);      // 测试线2

        m_renderer.DrawLine(m_vertices, vp);       // 测试线1

        m_renderer.DrawLine(m_vertices1, vp);      // 测试线2
          
        m_renderer.End();
    }

    
    void Viewport::Resize(float width, float height)
    {
		m_camera.Resize(width, height);
    }
     
    Camera& Viewport::GetCamera()
    {
        return m_camera;
    }

    const Camera& Viewport::GetCamera() const
    {
        return m_camera;
    }
      
    void Viewport::Pan(float dx, float dy)
    { 
        m_camera.Pan(dx, dy);  // 只平移，不缩放，不滚轮
        
    }

 
    void Viewport::Zoom(float delta, float mouseX, float mouseY)
    {
        m_camera.Zoom(delta, static_cast<int>(mouseX), static_cast<int>(mouseY));  // delta = 滚轮增量，mouseX/Y = 屏幕坐标
    }

}