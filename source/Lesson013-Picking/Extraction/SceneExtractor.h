#pragma once
#include <vector> 
#include "ISceneReader.h"
#include "Render/D3D11/Shader.h"
namespace MiniCAD
{  
    /// <summary>
	/// 可渲染数据的工具类（从 ISceneReader 读取数据，生成 RenderItem 和 RenderPreview）
    /// </summary>
    class SceneExtractor
    {
    public:
        // 主入口
        static void Extract(const ISceneReader& sceneReader, std::vector<Vertex_P3_C4>& sceneVerteies, std::vector<Vertex_P3_C4>& previewVerteies);

    private:
        // 类型提取
        static void ExtractEntityVerteies    (const ISceneReader& sceneReader, std::vector<Vertex_P3_C4>& sceneVerteies);
        static void ExtractPreviewVerteies  (const ISceneReader& sceneReader, std::vector<Vertex_P3_C4>& previewVerteies);
         
         
        // 核心几何生成
        static void BuildLine(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, XMFLOAT4& color, std::vector<Vertex_P3_C4>& out);

        static void BuildCircle(const DirectX::XMFLOAT3& center, float radius, int segments, XMFLOAT4& color, std::vector<Vertex_P3_C4>& out);

    };
}