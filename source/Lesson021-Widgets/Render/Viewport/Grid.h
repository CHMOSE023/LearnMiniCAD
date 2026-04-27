#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Camera.h"   
#include "Render/D3D11/Shader.h"  

namespace MiniCAD
{  
    class Grid
    {
    public:
        std::vector<Vertex_P3_C4> BuildGrid( const Camera& camera, bool showGrid, float screenWidth, float screenHeight)
        {
            std::vector<Vertex_P3_C4>   verts = {};

            XMFLOAT4 gridColor = { 0.15f, 0.20f, 0.25f, 1.0f };
            XMFLOAT4 gridColor5 = { 0.20f, 0.30f, 0.40f, 1.0f };

            XMFLOAT3 worldTL = camera.ScreenToWorld(0, 0);
            XMFLOAT3 worldBR = camera.ScreenToWorld(screenWidth, screenHeight);

            float worldLeft = worldTL.x;
            float worldRight = worldBR.x;
            float worldTop = worldTL.y;
            float worldBottom = worldBR.y;

            float pixelsPerUnit = screenWidth / (worldRight - worldLeft);
            float rawStep = 60.0f / pixelsPerUnit;
            float magnitude = std::pow(10.0f, std::floor(std::log10(rawStep)));
            float normalized = rawStep / magnitude;

            float step;
            if (normalized < 2.0f) step = magnitude;
            else if (normalized < 5.0f) step = magnitude * 2.0f;
            else                        step = magnitude * 5.0f;
            if (showGrid)
            {
                // 垂直网格线
                float startX = std::floor(worldLeft / step) * step;
                float endX = std::ceil(worldRight / step) * step;
                for (float wx = startX; wx <= endX; wx += step)
                {
                    int  index = (int)std::round(wx / step);
                    auto color = (index % 5 == 0) ? gridColor5 : gridColor;
                    XMFLOAT2 top = camera.WorldToScreen({ wx, worldTop, 0 });
                    verts.push_back({ {top.x, 0,            0}, color });
                    verts.push_back({ {top.x, screenHeight, 0}, color });
                }

                // 水平网格线
                float startY = std::floor(worldBottom / step) * step;
                float endY = std::ceil(worldTop / step) * step;
                for (float wy = startY; wy <= endY; wy += step)
                {
                    int  index = (int)std::round(wy / step);
                    auto color = (index % 5 == 0) ? gridColor5 : gridColor;
                    XMFLOAT2 left = camera.WorldToScreen({ worldLeft, wy, 0 });
                    verts.push_back({ {0,           left.y, 0}, color });
                    verts.push_back({ {screenWidth, left.y, 0}, color });
                }

               
             
               
            }

            // 原点 X 轴正半轴（红）
            XMFLOAT2 origin = camera.WorldToScreen({ 0, 0, 0 });
            if (origin.x < screenWidth && origin.y >= 0 && origin.y <= screenHeight)
            {
                verts.push_back({ {origin.x,    origin.y, 0}, {0.8f, 0.2f, 0.2f, 1} });
                verts.push_back({ {screenWidth, origin.y, 0}, {0.8f, 0.2f, 0.2f, 1} });
            }

            // 原点 Y 轴正半轴（绿）
            if (origin.y > 0 && origin.x >= 0 && origin.x <= screenWidth)
            {
                verts.push_back({ {origin.x, origin.y, 0}, {0.2f, 0.8f, 0.2f, 1} });
                verts.push_back({ {origin.x, 0,        0}, {0.2f, 0.8f, 0.2f, 1} });
            }

            // 坐标轴图标
            XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
            float len = 50.0f;
            float boxSize = 5.0f;
            float s = 15.0f;

            bool originVisible = origin.x >= 0 && origin.x <= screenWidth
                && origin.y >= 0 && origin.y <= screenHeight;
            float ax = originVisible ? origin.x : 30.0f;
            float ay = originVisible ? origin.y : screenHeight - 30.0f;

            // 原点小方框
            verts.push_back({ {ax - boxSize, ay - boxSize, 0}, color });
            verts.push_back({ {ax + boxSize, ay - boxSize, 0}, color });
            verts.push_back({ {ax + boxSize, ay - boxSize, 0}, color });
            verts.push_back({ {ax + boxSize, ay + boxSize, 0}, color });
            verts.push_back({ {ax + boxSize, ay + boxSize, 0}, color });
            verts.push_back({ {ax - boxSize, ay + boxSize, 0}, color });
            verts.push_back({ {ax - boxSize, ay + boxSize, 0}, color });
            verts.push_back({ {ax - boxSize, ay - boxSize, 0}, color });

            // X 轴
            verts.push_back({ {ax,       ay, 0}, color });
            verts.push_back({ {ax + len, ay, 0}, color });
            // 手绘 X
            float xx = ax + len + 4.0f, xy = ay - s;
            verts.push_back({ {xx,     xy,     0}, color });
            verts.push_back({ {xx + s, xy + s, 0}, color });
            verts.push_back({ {xx + s, xy,     0}, color });
            verts.push_back({ {xx,     xy + s, 0}, color });

            // Y 轴
            verts.push_back({ {ax, ay,       0}, color });
            verts.push_back({ {ax, ay - len, 0}, color });
            // 手绘 Y
            float yx = ax + 4.0f, yy = ay - len - s - 4.0f;
            verts.push_back({ {yx,         yy,         0}, color });
            verts.push_back({ {yx + s / 2,   yy + s / 2,   0}, color });
            verts.push_back({ {yx + s,     yy,         0}, color });
            verts.push_back({ {yx + s / 2,   yy + s / 2,   0}, color });
            verts.push_back({ {yx + s / 2,   yy + s / 2,   0}, color });
            verts.push_back({ {yx + s / 2,   yy + s,     0}, color });

            return verts;

        }
         
    };
}