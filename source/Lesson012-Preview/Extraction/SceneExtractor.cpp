#include "SceneExtractor.h"
#include <DirectXMath.h>
#include <cmath>
#include "Core/Entity/LineEntity.hpp"
using namespace DirectX;

namespace MiniCAD
{

    void SceneExtractor::Extract(const ISceneReader& sceneReader, std::vector<Vertex_P3_C4>& sceneVerteies, std::vector<Vertex_P3_C4>& previewVerteies)
    {
        sceneVerteies.clear();
        ExtractEntityVerteies(sceneReader, sceneVerteies);

        previewVerteies.clear();
        ExtractPreviewVerteies(sceneReader, previewVerteies);
         
    }

    void SceneExtractor::ExtractEntityVerteies(const ISceneReader& sceneReader, std::vector<Vertex_P3_C4>& items)
    {
        sceneReader.ForEachObject([&](const Object& obj)
        {   
                if (const LineEntity* line = dynamic_cast<const LineEntity*>(&obj))
                {  
					auto color = line->GetAttr().Color;
                    BuildLine(line->GetLine().Start, line->GetLine().End, color, items);
                }
                
		});        
    }   


    void SceneExtractor::ExtractPreviewVerteies(const ISceneReader& sceneReader, std::vector<Vertex_P3_C4>& items)
    {
        sceneReader.ForEachPreview([&](const Object& obj)
            { 
                if (const LineEntity* line = dynamic_cast<const LineEntity*>(&obj))
                {
                    auto color = line->GetAttr().Color;
                    BuildLine(line->GetLine().Start, line->GetLine().End, color, items);
                }

            });
    }


    // =======================
    // LineVerteies
    // =======================
    void SceneExtractor::BuildLine(const XMFLOAT3& p0, const XMFLOAT3& p1, XMFLOAT4& color, std::vector<Vertex_P3_C4>& out)
    {
        out.push_back({ p0, color });
        out.push_back({ p1, color });
    }

    // =======================
    // CircleVerteies
    // =======================
    void SceneExtractor::BuildCircle(const XMFLOAT3& center, float radius, int segments, XMFLOAT4& color, std::vector<Vertex_P3_C4>& out)
    {  
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

