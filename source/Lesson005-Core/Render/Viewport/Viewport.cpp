#include "Viewport.h"
#include <DirectXMath.h>
#include "Render/D3D11/Renderer.h"
#include <random>
using namespace DirectX;

namespace MiniCAD
{ 
    inline static XMFLOAT4 GetColor(int index, int maxIndex)
    {
        float t = (float)index / (float)maxIndex;

        //t = powf(t, 1.2f); // 非线性

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

    Viewport::Viewport(Renderer* renderer,float width, float height)
        : m_renderer(renderer) 
    {
        m_camera = std::make_unique<Camera>(width, height);  
         
        MiniCAD::FractalGeometryGenerator gen; 

        std::vector<MiniCAD::Line> lines;
        std::vector<MiniCAD::Line> lines1;

        // 谢尔宾斯基地毯
        MiniCAD::FractalGeometryGenerator::Rect r;
        r.min = { -7,0,0 };
        r.max = { -1,6,0 };

        gen.GenerateSierpinskiCarpet(r, 4, lines);

        // 递归分型
        MiniCAD::Line L0({ 0,0,0 }, { 6,0,0 });
        MiniCAD::Line L1({ 6,0,0 }, { 6,6,0 });
        MiniCAD::Line L2({ 6,6,0 }, { 0,6,0 });
        MiniCAD::Line L3({ 0,6,0 }, { 0,0,0 }); 
        gen.GenerateRecurseQuad(L0, L1, L2, L3, 50, lines1);
         
        // 转 GPU
        ConvertToVertices(lines, m_vertices); 
        ConvertToVertices(lines1, m_vertices1); 

    }
      
    void Viewport::Render(const RenderTarget& target)
    {
        if (!m_renderer || !m_camera) return;
         
        XMMATRIX vp = m_camera->GetViewProj();
         
        m_renderer->Begin(target);
        {
           // 绘制金刚石  
          
             
            // 屏幕矩阵
            XMMATRIX screenVP = XMMatrixOrthographicOffCenterLH(
                0.0f, target.viewport.Width,
                target.viewport.Height, 0.0f,
                0.0f, 1.0f
            );

            m_renderer->DrawLine(m_vertices, vp);
            m_renderer->DrawLine(m_vertices1, vp);

            // 矩形左上(50,50) 右下(150,150)
            //std::vector<Vertex_P3> verts1 = {};
            //verts1.push_back({ {  50.0f,  50.0f, 0.0f} }); // 左上
            //verts1.push_back({ { 150.0f,  50.0f, 0.0f} }); // 右上
            //verts1.push_back({ {  50.0f, 150.0f, 0.0f} }); // 左下

            //verts1.push_back({ { 150.0f,  50.0f, 0.0f} }); // 右上
            //verts1.push_back({ { 150.0f, 150.0f, 0.0f} }); // 右下
            //verts1.push_back({ {  50.0f, 150.0f, 0.0f} }); // 左下;  
            //m_renderer->DrawGrip(verts1, { 0.1f, 0.5f, 1.0f, 0.6f }, screenVP);    

           // m_renderer->DrawGrip(g_petalFan, { 0.3f, 0.7f, 0.1f, 0.7 }, screenVP);
        }
         
        m_renderer->End();
    }

    void Viewport::Resize(float width, float height)
    {
		m_camera->Resize(width, height);
    }
     
    Camera* Viewport::GetCamera() const
    {
        return m_camera.get();
    }

      
    void Viewport::Pan(float dx, float dy)
    { 
        if (m_camera)
        {
            m_camera->Pan(dx, dy );  // 只平移，不缩放，不滚轮
        }
    }

 
    void Viewport::Zoom(float delta, float mouseX, float mouseY)
    {
        if (m_camera)
        { 
            m_camera->Zoom(delta, static_cast<int>(mouseX), static_cast<int>(mouseY));  // delta = 滚轮增量，mouseX/Y = 屏幕坐标
        }
    }

}