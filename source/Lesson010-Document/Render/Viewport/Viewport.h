#pragma once 
#include "Render/Resources/RenderItem.h"
#include "Render/Resources/RenderPreview.h"
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Camera.h"
#include "Render/Viewport/Grid.h"   
#include "Extraction/ISceneReader.h"
#include <unordered_set> 
namespace MiniCAD
{ 
    class Viewport
    {
    public:
        Viewport(Renderer* renderer, float width, float height);

        void Render(const RenderTarget& target);

        void Resize(float width, float height);

        void SetCamera(Camera* camera);

        void RefreshRenderData(const ISceneReader& scene);
        
    private:
        std::unique_ptr<Grid>    m_grid;
        Camera*                  m_camera;
        Renderer*                m_renderer; 

    private:
        std::vector<Vertex_P3_C4>  m_sceneVerteies; 

        std::vector<RenderItem>    m_cachedItems;
        std::vector<RenderPreview> m_cachedPreviews;
    };
	
}
