#include "Viewport.h"
#include <DirectXMath.h>
#include "Render/D3D11/Renderer.h"  
#include "Camera.h"
using namespace DirectX;

namespace MiniCAD
{ 
    Viewport::Viewport(Renderer& renderer, float width, float height)
        : m_renderer(renderer)
        , m_camera(width, height)
    { 

    }
     
    void Viewport::Render(const RenderTarget& target, ViewState&  viewState)
    {
        XMMATRIX vp = m_camera.GetViewProj();

        m_renderer.BeginFrame(target);

        if (viewState.Selection.Active) // 选择框
        {
            auto sel = BuildSelectionGeometry(target, viewState);

            m_renderer.Submit(sel.fill, sel.screenVP, PrimitiveType::Triangle, true, true);
            m_renderer.Submit(sel.border, sel.screenVP, PrimitiveType::Line, true, true);
        }

        m_renderer.Submit(viewState.Scene, vp, PrimitiveType::Line, true, true);
        m_renderer.Submit(viewState.Overlay, vp, PrimitiveType::Line, true, true);

        m_renderer.EndFrame();


    }

    void Viewport::Resize(float width, float height)
    {
        m_camera.Resize(width, height);
    }

    Camera& Viewport::GetCamera()
    {
        return m_camera;
    }

    const Camera& Viewport::GetCamera() const
    {
        return m_camera;
    }

    void Viewport::Pan(float dx, float dy)
    {
        m_camera.Pan(dx, dy);  // 只平移，不缩放，不滚轮

    }


    void Viewport::Zoom(float delta, float mouseX, float mouseY)
    {
        m_camera.Zoom(delta, static_cast<int>(mouseX), static_cast<int>(mouseY));  // delta = 滚轮增量，mouseX/Y = 屏幕坐标
    }

    // 构建选择框
    SelectionGeometry Viewport::BuildSelectionGeometry(const RenderTarget& target, ViewState& vs)
    {
        SelectionGeometry g;

        int x0 = vs.Selection.Start.x;
        int y0 = vs.Selection.Start.y;
        int x1 = vs.Selection.End.x;
        int y1 = vs.Selection.End.y;

        float left   = std::min(x0, x1);
        float right  = std::max(x0, x1);
        float top    = std::min(y0, y1);
        float bottom = std::max(y0, y1);

        bool cross = (x1 < x0);

        DirectX::XMFLOAT4 fillColor =
            cross ? DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.25f)
            : DirectX::XMFLOAT4(0.0f, 0.4f, 1.0f, 0.25f);

        DirectX::XMFLOAT4 lineColor =
            cross ? DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)
            : DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

        // fill
        g.fill = {
            {{left,  top,    0}, fillColor},
            {{right, top,    0}, fillColor},
            {{right, bottom, 0}, fillColor},

            {{left,  top,    0}, fillColor},
            {{right, bottom, 0}, fillColor},
            {{left,  bottom, 0}, fillColor},
        };

        DirectX::XMFLOAT3 p0{ left,  top,    0 };
        DirectX::XMFLOAT3 p1{ right, top,    0 };
        DirectX::XMFLOAT3 p2{ right, bottom, 0 };
        DirectX::XMFLOAT3 p3{ left,  bottom, 0 };

        // border
        if (cross)
        {
            AddDashedLine(g.border, p0, p1, lineColor);
            AddDashedLine(g.border, p1, p2, lineColor);
            AddDashedLine(g.border, p2, p3, lineColor);
            AddDashedLine(g.border, p3, p0, lineColor);
        }
        else
        {
            g.border = {
                {p0, lineColor}, {p1, lineColor},
                {p1, lineColor}, {p2, lineColor},
                {p2, lineColor}, {p3, lineColor},
                {p3, lineColor}, {p0, lineColor},
            };
        }

        g.screenVP = XMMatrixOrthographicOffCenterLH(0.0f, target.viewport.Width, target.viewport.Height, 0.0f, 0.0f, 1.0f);

        return g;
    }

    // 添加点画线
    void Viewport::AddDashedLine(std::vector<Vertex_P3_C4>& out, XMFLOAT3& a, XMFLOAT3& b, XMFLOAT4& color, float dashLen, float gapLen)
    {
        using namespace DirectX;

        XMVECTOR A = XMLoadFloat3(&a);
        XMVECTOR B = XMLoadFloat3(&b);

        XMVECTOR dir = B - A;
        float len = XMVectorGetX(XMVector3Length(dir));

        if (len < 0.001f) return;

        dir = XMVector3Normalize(dir);

        float t = 0.0f;

        while (t < len)
        {
            float t2 = std::min(t + dashLen, len);

            XMVECTOR p0 = A + dir * t;
            XMVECTOR p1 = A + dir * t2;

            DirectX::XMFLOAT3 p0f, p1f;
            XMStoreFloat3(&p0f, p0);
            XMStoreFloat3(&p1f, p1);

            out.push_back({ p0f, color });
            out.push_back({ p1f, color });

            t += dashLen + gapLen;
        }
    }

}