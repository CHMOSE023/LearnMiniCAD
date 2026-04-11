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
         
        m_renderer->Begin(target, vp);
  
        // 绘制一个三角形
        LineVertex tri[3] =
        {
            {{ 0.0f,  0.5f, 0.0f}, {1,0,0,1}},  // 顶部 红
            {{ 0.5f, -0.5f, 0.0f}, {0,1,0,1}},  // 右下 绿
            {{-0.5f, -0.5f, 0.0f}, {0,0,1,1}},  // 左下 蓝
        };
         
        m_renderer->Submit(tri, 3);
         
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