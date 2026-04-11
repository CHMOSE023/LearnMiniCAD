#include "Viewport.h"
#include <DirectXMath.h>
#include "Render/D3D11/Renderer.h"
using namespace DirectX;

namespace MiniCAD
{

    Viewport::Viewport(Renderer* renderer,float width, float height)
        : m_renderer(renderer) 
    {
        m_camera = std::make_unique<Camera>(width, height);  
    }

  
    void Viewport::Render(const RenderTarget& target)
    {
        if (!m_renderer || !m_camera) return;
         
        XMMATRIX vp = m_camera->GetViewProj();
         
        m_renderer->Begin(target);

        {
            // 绘制在世界坐标
            std::vector<Vertex_P3_C4> verts = {};
            verts.push_back({ { 0.0f,  0.5f, 0.0f}, {1,0,0,1} });
            verts.push_back({ { 0.5f, -0.5f, 0.0f}, {0,1,0,1} });
            verts.push_back({ {-0.5f, -0.5f, 0.0f}, {0,0,1,1} });
            
            m_renderer->DrawLine(verts, vp);

            std::vector<Vertex_P3> verts1 = {};
             
            // 屏幕矩阵
            XMMATRIX screenVP = XMMatrixOrthographicOffCenterLH(
                0.0f, target.viewport.Width,
                target.viewport.Height, 0.0f,
                0.0f, 1.0f
            );

            // 矩形左上(50,50) 右下(150,150)
            verts1.push_back({ {  50.0f,  50.0f, 0.0f} }); // 左上
            verts1.push_back({ { 150.0f,  50.0f, 0.0f} }); // 右上
            verts1.push_back({ {  50.0f, 150.0f, 0.0f} }); // 左下

            verts1.push_back({ { 150.0f,  50.0f, 0.0f} }); // 右上
            verts1.push_back({ { 150.0f, 150.0f, 0.0f} }); // 右下
            verts1.push_back({ {  50.0f, 150.0f, 0.0f} }); // 左下; 
           
            m_renderer->DrawGrip(verts1, { 0.1f, 0.5f, 1.0f, 0.6f }, screenVP);


            // 第二个矩形 平移100 左上(150,150) 右下(250,250)
            verts1.clear();
            verts1.push_back({ { 150.0f, 150.0f, 0.0f} });
            verts1.push_back({ { 250.0f, 150.0f, 0.0f} });
            verts1.push_back({ { 150.0f, 250.0f, 0.0f} });

            verts1.push_back({ { 250.0f, 150.0f, 0.0f} });
            verts1.push_back({ { 250.0f, 250.0f, 0.0f} });
            verts1.push_back({ { 150.0f, 250.0f, 0.0f} });
            m_renderer->DrawGrip(verts1, { 0.8f, 0.2f, 0.8f, 0.6f }, screenVP);


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