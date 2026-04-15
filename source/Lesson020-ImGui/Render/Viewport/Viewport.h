#pragma once 
#include "Render/Resources/RenderItem.h"
#include "Render/Resources/RenderPreview.h"
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Camera.h"
#include "Render/Viewport/Grid.h"   
#include "Render/Viewport/Cursor.h"   
#include "Extraction/ISceneReader.h"
#include <unordered_set> 
#include "Render/ViewState.h"
namespace MiniCAD
{ 
    class Viewport
    {
    public:
        Viewport(Renderer* renderer, float width, float height);

        void Render(const RenderTarget& target);  

        void Resize(float width, float height);

        void SetCamera(Camera* camera);

        void RefreshRenderData(const ISceneReader& scene, const ViewState& viewState);
        
    private:
        void RenderScenePass       (const RenderTarget& target);
        void RenderOverlayPass     (const RenderTarget& target);
        void RenderGridPass        (const RenderTarget& target);
        void BuildOverlayGeometry  (const ViewState&    viewState); 
    private:

        Grid       m_grid;
        Cursor     m_cursor;

        Camera*    m_camera;
        Renderer*  m_renderer;    

        std::vector<Vertex_P3_C4>  m_sceneVerteies;       
        std::vector<Vertex_P3_C4>  m_previewVerts;  
        std::vector<Vertex_P3_C4>  m_overlayVerts;
        std::vector<Vertex_P3_C4>  m_gridVerts;

        std::vector<RenderItem>    m_cachedItems;
        std::vector<RenderPreview> m_cachedPreviews;

		float  m_width = 0.f;
		float  m_height = 0.f;
         
    };
	
}
