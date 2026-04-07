#include "Viewport.h"
#include <DirectXMath.h>
#include "Render/Extraction/RenderExtractor.h"
using namespace DirectX;

namespace MiniCAD
{

    Viewport::Viewport(Renderer* renderer,float width, float height)
        : m_renderer(renderer) 
    {
        m_camera = std::make_unique<Camera>(width, height);  
        m_grid   = std::make_unique<Grid>(m_camera->GetCameraPos());
    }

    void Viewport::Render(const RenderTarget& target, const ISceneReader& scene)
    {
        if (!m_renderer || !m_camera) return;
         
        XMMATRIX vp = m_camera->GetViewProj();
         
        m_renderer->Begin(target, vp);
  
        //  直接用缓存数据
        for (auto& preview : m_cachedPreviews)
        {
            m_renderer->Submit(preview.vertices.data(), preview.vertices.size());
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

    void Viewport::RefreshRenderData(const ISceneReader& scene)
    {
        m_cachedItems.clear();
        m_cachedPreviews.clear();
        RenderExtractor::Extract(scene, m_cachedItems, m_cachedPreviews);
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