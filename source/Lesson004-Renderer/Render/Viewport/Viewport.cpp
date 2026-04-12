#include "Viewport.h"
#include <DirectXMath.h>
#include "Render/D3D11/Renderer.h"
#include <random>
using namespace DirectX;

namespace MiniCAD
{

    std::vector<Vertex_P3_C4> BuildVertex()
    {
       const  int n = 20;  // 等分点数
       const  int r = 4;  // 半径       

        XMFLOAT3 pts   [n];
        XMFLOAT4 colors[n];

        double theta = 2 * 3.1415926 / n;   // 定义金刚石图案的等分角

        
        static std::mt19937 rng(12345);
        std::uniform_real_distribution<float> dist(0.4f, 0.9f); // 避免太暗

        for (int i = 0; i < n; i++)         // 计算等分点坐标
        { 
            pts[i].x = r * cos(i * theta);
            pts[i].y = r * sin(i * theta);
            pts[i].z = 0;

            // 随机颜色（RGB）
            colors[i].x = dist(rng); // R
            colors[i].y = 0.3; // G
            colors[i].z = 0.1; // B
            colors[i].w = 1.0f;
        }

        std::vector<Vertex_P3_C4> lines; 

        for (int i = 0; i < n - 1; i++) //直线链接等分点
        {
            for (int j = i + 1; j < n; j++)
            {
                Vertex_P3_C4 start;
                start.pos = pts[i];
                start.color = colors[i];
                lines.push_back(start);

                Vertex_P3_C4 end;
                end.pos = pts[j];
                end.color = colors[j];
                lines.push_back(end);
            }
        }

        return lines;
    }
     
    std::vector<Vertex_P3> BuildPetalFan(float cx, float cy, float radius, int petalCount, float amplitude, int segments)
    {
        std::vector<Vertex_P3> verts;
        verts.reserve(segments * 3);

        float start = 0.0f;
        float end = XM_2PI;
        float step = (end - start) / segments;

        XMFLOAT3 center = { cx, cy, 0.0f };

        for (int i = 0; i < segments; i++)
        {
            float a0 = start + i * step;
            float a1 = start + (i + 1) * step;

            // 核心：半径扰动
            float r0 = radius * (1.0f + amplitude * sinf(petalCount * a0));
            float r1 = radius * (1.0f + amplitude * sinf(petalCount * a1));

            XMFLOAT3 p0 = { cx + r0 * cosf(a0),cy + r0 * sinf(a0),0.0f };

            XMFLOAT3 p1 = { cx + r1 * cosf(a1),cy + r1 * sinf(a1),0.0f };

            verts.push_back({ center });
            verts.push_back({ p0 });
            verts.push_back({ p1 });
        }

        return verts;
    }

    Viewport::Viewport(Renderer* renderer,float width, float height)
        : m_renderer(renderer) 
    {
        m_camera = std::make_unique<Camera>(width, height);  
    }


    std::vector<Vertex_P3>  g_petalFan = BuildPetalFan(
        120, 120,       // 中心
        60.0,           // 基础半径
        5,              // 花瓣数
        0.12f,          // 振幅（越大越夸张）
        128             // 分段
    );
   
    std::vector<Vertex_P3_C4> g_verts = BuildVertex();
    void Viewport::Render(const RenderTarget& target)
    {
        if (!m_renderer || !m_camera) return;
         
        XMMATRIX vp = m_camera->GetViewProj();
         
        m_renderer->Begin(target);
        {
           // 绘制金刚石 

           m_renderer->DrawLine(g_verts, vp);
             
            // 屏幕矩阵
            XMMATRIX screenVP = XMMatrixOrthographicOffCenterLH(
                0.0f, target.viewport.Width,
                target.viewport.Height, 0.0f,
                0.0f, 1.0f
            );

            // 矩形左上(50,50) 右下(150,150)
            //std::vector<Vertex_P3> verts1 = {};
            //verts1.push_back({ {  50.0f,  50.0f, 0.0f} }); // 左上
            //verts1.push_back({ { 150.0f,  50.0f, 0.0f} }); // 右上
            //verts1.push_back({ {  50.0f, 150.0f, 0.0f} }); // 左下

            //verts1.push_back({ { 150.0f,  50.0f, 0.0f} }); // 右上
            //verts1.push_back({ { 150.0f, 150.0f, 0.0f} }); // 右下
            //verts1.push_back({ {  50.0f, 150.0f, 0.0f} }); // 左下;  
            //m_renderer->DrawGrip(verts1, { 0.1f, 0.5f, 1.0f, 0.6f }, screenVP);    

            m_renderer->DrawGrip(g_petalFan, { 0.3f, 0.7f, 0.1f, 0.7 }, screenVP);
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