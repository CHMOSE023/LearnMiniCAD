#include "RenderExtractor.h"
#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

namespace MiniCAD
{

    void RenderExtractor::Extract(const ISceneReader& sceneReader, std::vector<RenderItem>& items, std::vector<RenderPreview>& previews)
    {
        items.clear();    
        previews.clear();   

        ExtractLines(sceneReader, previews);
        ExtractCircles(sceneReader, previews);
         
    }

    // =======================
    // Line
    // =======================
    void RenderExtractor::ExtractLines(const ISceneReader& sceneReader, std::vector<RenderPreview>& previews)
    {
        //for (const auto& line : sceneReader.GetLines())
        //{
        //    RenderPreview preview;
        //    // BuildLine(line.p0, line.p1, preview.vertices);
        //    previews.push_back(std::move(preview));
        //}
    }

    void RenderExtractor::BuildLine(const XMFLOAT3& p0, const XMFLOAT3& p1, std::vector<Vertex_P3_C4>& out)
    {
        XMFLOAT4 color = { 1.f, 1.f, 1.f, 1.f };
        out.push_back({ p0, color });
        out.push_back({ p1, color });
    }

    // =======================
    // Circle
    // =======================
    void RenderExtractor::ExtractCircles(const ISceneReader& sceneReader,  std::vector<RenderPreview>& previews)
    {
        //for (const auto& circle : sceneReader.GetCircles())
        //{
        //    RenderPreview preview;
        //    // BuildCircle(circle.center, circle.radius, 64, preview.vertices);
        //    previews.push_back(std::move(preview));
        //}
    }

    void RenderExtractor::BuildCircle(const XMFLOAT3& center,  float radius, int segments, std::vector<Vertex_P3_C4>& out)
    {
        XMFLOAT4 color = { 0.7f, 0.7f, 1.f, 1.f };

        for (int i = 0; i < segments; i++)
        {
            float a0 = (float)i / segments * XM_2PI;
            float a1 = (float)(i + 1) / segments * XM_2PI;

            XMFLOAT3 p0 = { center.x + cosf(a0) * radius, center.y + sinf(a0) * radius,  center.z };
            XMFLOAT3 p1 = { center.x + cosf(a1) * radius,  center.y + sinf(a1) * radius,  center.z };

            out.push_back({ p0, color });
            out.push_back({ p1, color });
        }
    }

}  

