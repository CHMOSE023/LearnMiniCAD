#pragma once

#include <vector>
#include <DirectXMath.h>
#include "Core/GeomKernel/Line.hpp"

using namespace DirectX;

namespace MiniCAD
{

    class FractalGeometryGenerator
    {
    public:

        struct Rect
        {
            XMFLOAT3 min;
            XMFLOAT3 max;
        };

        // =========================================================
        // Sierpinski Carpet  谢尔宾斯基地毯
        // =========================================================
        void GenerateSierpinskiCarpet(const Rect& r, int depth, std::vector<Line>& out)
        {
            if (depth <= 0)
            {
                AddRect(r, out);
                return;
            }

            float dx = (r.max.x - r.min.x) / 3.0f;
            float dy = (r.max.y - r.min.y) / 3.0f;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    if (i == 1 && j == 1)
                        continue;

                    Rect sub;
                    sub.min = {
                        r.min.x + i * dx,
                        r.min.y + j * dy,
                        0
                    };

                    sub.max = {
                        r.min.x + (i + 1) * dx,
                        r.min.y + (j + 1) * dy,
                        0
                    };

                    GenerateSierpinskiCarpet(sub, depth - 1, out);
                }
            }
        }

        /// <summary>
        /// 科赫雪花边
        /// </summary> 
        void GenerateKochCurve(const Line& line, int depth, std::vector<Line>& out)
        {
            if (depth <= 0)
            {
                out.push_back(line);
                return;
            }

            const XMFLOAT3& A = line.Start;
            const XMFLOAT3& B = line.End;

            auto Lerp = [](const XMFLOAT3& a, const XMFLOAT3& b, float t)
                {
                    return XMFLOAT3{
                        a.x + (b.x - a.x) * t,
                        a.y + (b.y - a.y) * t,
                        0
                    };
                };

            // =========================
            // Midpoint 核心使用
            // =========================
            XMFLOAT3 P1 = Lerp(A, B, 1.0f / 3.0f);
            XMFLOAT3 P2 = Lerp(A, B, 0.5f);   // Midpoint（关键）
            XMFLOAT3 P3 = Lerp(A, B, 2.0f / 3.0f);

            // =========================
            // 构造“尖峰”
            // =========================
            XMFLOAT3 dir = {
                B.x - A.x,
                B.y - A.y,
                0
            };

            // 2D 旋转 60°
            float h = 0.577f; // tan(30°)

            XMFLOAT3 peak = {
                P2.x - dir.y * h,
                P2.y + dir.x * h,
                0
            };

            // =========================
            // 分裂成4段
            // =========================
            Line L1(A, P1);
            Line L2(P1, peak);
            Line L3(peak, P3);
            Line L4(P3, B);

            GenerateKochCurve(L1, depth - 1, out);
            GenerateKochCurve(L2, depth - 1, out);
            GenerateKochCurve(L3, depth - 1, out);
            GenerateKochCurve(L4, depth - 1, out);
        }
        // =========================================================
        // Recurse Quad 递归四元
        // =========================================================
        void GenerateRecurseQuad(const Line& L0, const Line& L1, const Line& L2, const Line& L3, int depth, std::vector<Line>& out)
        {
           
            if (depth <= 0)
                return;

            auto Step = [](const Line& L) {  return L.PointAt(0.93f);  };
             
            XMFLOAT3 P0, P1, P2, P3;

            XMStoreFloat3(&P0, Step(L0));
            XMStoreFloat3(&P1, Step(L1));
            XMStoreFloat3(&P2, Step(L2));
            XMStoreFloat3(&P3, Step(L3));

            // =========================
            // 构造下一层
            // =========================
            Line NL0(P0, P1);
            Line NL1(P1, P2);
            Line NL2(P2, P3);
            Line NL3(P3, P0);

            
            out.push_back(L0);
            out.push_back(L1);
            out.push_back(L2);
            out.push_back(L3);

            // =========================
            // 下一层
            // =========================
            GenerateRecurseQuad(NL0, NL1, NL2, NL3, depth - 1, out);
        }

    private: 
        // =========================================================
        // Rect → 4 Lines
        // =========================================================
        void AddRect(const Rect& r, std::vector<Line>& out)
        {
            XMFLOAT3 a = { r.min.x, r.min.y, 0 };
            XMFLOAT3 b = { r.max.x, r.min.y, 0 };
            XMFLOAT3 c = { r.max.x, r.max.y, 0 };
            XMFLOAT3 d = { r.min.x, r.max.y, 0 };

            out.emplace_back(a, b);
            out.emplace_back(b, c);
            out.emplace_back(c, d);
            out.emplace_back(d, a);
        }
    };

}  
