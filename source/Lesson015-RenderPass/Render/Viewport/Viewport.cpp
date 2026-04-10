#include "Viewport.h"
#include <DirectXMath.h>
#include "Extraction/SceneExtractor.h"
using namespace DirectX;

namespace MiniCAD
{ 
    Viewport::Viewport(Renderer* renderer,float width, float height)
        : m_renderer(renderer)
		, m_camera(nullptr)
		, m_width(width)
		, m_height(height)
    { 
    }

    void Viewport::Render(const RenderTarget& target)
    {
        if (!m_renderer || !m_camera) return;       
         
        m_renderer->BeginFrame(target);
        {
			RenderGridPass(target);
            RenderScenePass(target);
            RenderOverlayPass(target);
        }
        m_renderer->EndFrame();
    }

    void Viewport::Resize(float width, float height)
    {
        m_width  = width;
        m_height = height; 
    } 

    void Viewport::SetCamera(Camera* camera)
    {
		m_camera = camera;
    } 

    void Viewport::RefreshRenderData(const ISceneReader& scene, const ViewState& viewState)
    {
        m_sceneVerteies.clear();
		m_previewVerts.clear(); 

        BuildOverlayGeometry(viewState);

        SceneExtractor::Extract(scene, viewState, m_sceneVerteies, m_previewVerts);        
    }
     
    void Viewport::RenderScenePass(const RenderTarget& target)
    {
        RenderPassDesc pass = {};
        pass.ViewProj = m_camera->GetViewProj();
        pass.Depth    = true;

        m_renderer->BeginPass(pass);
        m_renderer->DrawLine(m_sceneVerteies);
        m_renderer->DrawLine(m_previewVerts);
        m_renderer->EndPass();
    }

    void Viewport::RenderOverlayPass(const RenderTarget& target)
    {
        RenderPassDesc pass = {};
        float width  = target.viewport.Width;
        float height = target.viewport.Height;

        pass.ViewProj = XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f,-1.0f, 1.0f);
        pass.Depth    = false;
          

        m_renderer->BeginPass(pass);
        m_renderer->DrawLine(m_overlayVerts); 
        m_renderer->EndPass();
    }

    void Viewport::RenderGridPass(const RenderTarget& target)
    {
        RenderPassDesc pass = {};
        float width  = target.viewport.Width;
        float height = target.viewport.Height;

        pass.ViewProj = XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
        pass.Depth = false;

        m_renderer->BeginPass(pass);
        m_renderer->DrawLine(m_gridVerts);  // 单独存网格顶点
        m_renderer->EndPass();
    }

    void Viewport::BuildOverlayGeometry(const ViewState& state)
    {
        m_overlayVerts.clear();   
        m_gridVerts.clear();
        // 网格 + 坐标轴 + 图标
        m_grid.Build(m_camera, m_width, m_height);
        auto& gridVerts = m_grid.GetVerts();
        m_gridVerts.insert(m_gridVerts.end(), gridVerts.begin(), gridVerts.end());
         
        // 一、 鼠标位置十字线
        m_cursor.Build(state, m_width, m_height);
        auto& cursorVerts = m_cursor.GetVerts();
        m_overlayVerts.insert(m_overlayVerts.end(), cursorVerts.begin(), cursorVerts.end());
          

    }
      
}