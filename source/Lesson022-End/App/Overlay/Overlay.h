#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "Core/Object/Object.hpp"
#include "Render/D3D11/Shader.h" 

namespace MiniCAD
{
    /// <summary>
    /// “非持久、仅用于显示”的临时几何数据
    /// </summary>
    class Overlay
    {
    public:
        // 清空
        void Clear()
        {
            m_lines.clear();
            m_points.clear();
        } 

        // ===== Line =====
        void AddLine(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT4& color)
        {
            m_lines.push_back({ a, b, color });
        }

        // ===== Point =====
        void AddPoint(const XMFLOAT3& p, const XMFLOAT4& color)
        {
            m_points.push_back({ p, color });
        }

        // ===== Export to GPU vertices =====
        void ToVertices(std::vector<Vertex_P3_C4>& out) const
        {
            out.reserve(out.size() + m_lines.size() * 2 + m_points.size());

            // Lines -> 2 vertices
            for (const auto& l : m_lines)
            {
                out.push_back({ l.a, l.color });
                out.push_back({ l.b, l.color });
            }

            // Points -> single vertex
            for (const auto& p : m_points)
            {
                out.push_back({ p.p, p.color });
            }
        }

        bool Empty() const
        {
            return m_lines.empty() && m_points.empty();
        }
         

    private:
        struct Line
        {
            XMFLOAT3 a;
            XMFLOAT3 b;
            XMFLOAT4 color;
        };

        struct Point
        {
            XMFLOAT3 p;
            XMFLOAT4 color;
        };

    private:
        std::vector<Line>  m_lines;
        std::vector<Point> m_points;
    }; 
}