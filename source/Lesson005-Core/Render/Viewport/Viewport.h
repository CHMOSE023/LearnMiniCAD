#pragma once 
#include "Render/Resources/RenderItem.h"
#include "Render/Resources/RenderPreview.h"
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Camera.h"
#include "Render/Viewport/Grid.h"   
#include "Interfaces/ISceneReader.h"
#include <unordered_set> 
namespace MiniCAD
{ 
    class Viewport
    {
    public:
        Viewport(Renderer* renderer, float width, float height);

        void Render(const RenderTarget& target, const ISceneReader& scene);

        void Resize(float width, float height);

        Camera* GetCamera() const;

        void RefreshRenderData(const ISceneReader& scene);
        // 交互 
        void Pan (float dx, float dy);
        void Zoom(float delta, float mouseX, float mouseY);
    private:
        std::unique_ptr<Camera>  m_camera;
        std::unique_ptr<Grid>    m_grid;
        Renderer*                m_renderer; 

    private:
        std::vector<RenderItem>    m_cachedItems;
        std::vector<RenderPreview> m_cachedPreviews;
    };
	
}
