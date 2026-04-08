#pragma once
#include <vector>
#include "Render/Resources/RenderPreview.h"
#include "Render/Resources/RenderItem.h"
#include "Interfaces/ISceneReader.h"
namespace MiniCAD
{ 

    class RenderExtractor
    {
    public:
        // 主入口
        static void Extract(const ISceneReader& sceneReader, std::vector<RenderItem>& items, std::vector<RenderPreview>& previews);

    private:
        // 类型提取
        static void ExtractLines  (const ISceneReader& sceneReader, std::vector<RenderPreview>& previews); 

        static void ExtractCircles(const ISceneReader& sceneReader, std::vector<RenderPreview>& previews);

    private:
        // 核心几何生成
        static void BuildLine(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, std::vector<Vertex_P3_C4>& out);

        static void BuildCircle(const DirectX::XMFLOAT3& center, float radius, int segments, std::vector<Vertex_P3_C4>& out);

    };
}