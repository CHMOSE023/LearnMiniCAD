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
         
        // 鼠标位置十字线
        m_cursor.Build(state, m_width, m_height);
        auto& cursorVerts = m_cursor.GetVerts();
        m_overlayVerts.insert(m_overlayVerts.end(), cursorVerts.begin(), cursorVerts.end());
        
		// snap 点 
        if (state.Snap.IsValid())
        { 
            float x = state.Snap.Pos.x;
            float y = state.Snap.Pos.y;
            switch (state.Snap.SnapType)
            {
            case SnapDraw::Type::Endpoint:
            {
                // 黄色方框，比夹点稍大
                const float size = 7.0f;
                DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
                m_overlayVerts.push_back({ {x - size, y - size, 0}, color });
                m_overlayVerts.push_back({ {x + size, y - size, 0}, color });
                m_overlayVerts.push_back({ {x + size, y - size, 0}, color });
                m_overlayVerts.push_back({ {x + size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x + size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x - size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x - size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x - size, y - size, 0}, color });
                break;
            }
            case SnapDraw::Type::Midpoint:
            {
                // 黄色三角形
                const float size = 7.0f;
                DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
                m_overlayVerts.push_back({ {x,      y - size, 0}, color });
                m_overlayVerts.push_back({ {x + size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x + size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x - size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x - size, y + size, 0}, color });
                m_overlayVerts.push_back({ {x,      y - size, 0}, color });
                break;
            }
            case SnapDraw::Type::Nearest:
            {
                // 黄色十字
                const float size = 7.0f;
                DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 0.0f, 1.0f };
                m_overlayVerts.push_back({ {x - size, y, 0}, color });
                m_overlayVerts.push_back({ {x + size, y, 0}, color });
                m_overlayVerts.push_back({ {x, y - size, 0}, color });
                m_overlayVerts.push_back({ {x, y + size, 0}, color });
                break;
            }
            default: break; // Grid 不画
            }
		}


        // 选中的夹点
        for (const auto& g : state.Grips)
        {
            const float size = 4.5f;

            float x = g.Pos.x;
            float y = g.Pos.y;

            DirectX::XMFLOAT4 color = { 0.8f, 0.8f, 0.8f, 1.0f }; // 橙色（拖拽）
            if (g.Hovered)
            {
                color = { 1.0f, 0.85f, 0.1f, 1.0f }; // 黄色（悬停）
            }
            
            // 四条线组成一个小方框
            if (g.GripType == GripDraw::Type::Start || g.GripType == GripDraw::Type::End)
            {
                // square
                m_overlayVerts.push_back({ {x - size, y - size,0}, color });
                m_overlayVerts.push_back({ {x + size, y - size,0}, color });

                m_overlayVerts.push_back({ {x + size, y - size,0}, color });
                m_overlayVerts.push_back({ {x + size, y + size,0}, color });

                m_overlayVerts.push_back({ {x + size, y + size,0}, color });
                m_overlayVerts.push_back({ {x - size, y + size,0}, color });

                m_overlayVerts.push_back({ {x - size, y + size,0}, color });
                m_overlayVerts.push_back({ {x - size, y - size,0}, color });
            }
            else // Mid → circle（近似）
            {
                const int seg = 12;
                float r = size+1.0f;

                for (int i = 0; i < seg; i++)
                {
                    float a0 = (i / (float)seg) * DirectX::XM_2PI;
                    float a1 = ((i + 1) / (float)seg) * DirectX::XM_2PI;

                    float x0 = x + cosf(a0) * r;
                    float y0 = y + sinf(a0) * r;
                    float x1 = x + cosf(a1) * r;
                    float y1 = y + sinf(a1) * r;

                    m_overlayVerts.push_back({ {x0,y0,0}, color });
                    m_overlayVerts.push_back({ {x1,y1,0}, color });
                }
            }
        }
    }
      
}