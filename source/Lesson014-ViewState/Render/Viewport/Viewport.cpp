#include "Viewport.h"
#include <DirectXMath.h>
#include "Extraction/SceneExtractor.h"
using namespace DirectX;

namespace MiniCAD
{ 
    Viewport::Viewport(Renderer* renderer,float width, float height)
        : m_renderer(renderer)
		, m_camera(nullptr)
    { 
    }

    void Viewport::Render(const RenderTarget& target)
    {
        if (!m_renderer || !m_camera) return;
         
        XMMATRIX vp = m_camera->GetViewProj();
         
        m_renderer->Begin(target, vp);
   
        m_renderer->DrawLine(m_sceneVerteies);
        m_renderer->DrawLine(m_previewVerts);
         
        m_renderer->End();
    }

    void Viewport::Resize(float width, float height)
    {
		m_camera->Resize(width, height);
    }
     

    void Viewport::SetCamera(Camera* camera)
    {
		m_camera = camera;
    }
      

    void Viewport::RefreshRenderData(const ISceneReader& scene, const ViewState& viewState)
    {
        m_sceneVerteies.clear();
		m_previewVerts.clear();
        SceneExtractor::Extract(scene, viewState, m_sceneVerteies, m_previewVerts);         
    } 

}