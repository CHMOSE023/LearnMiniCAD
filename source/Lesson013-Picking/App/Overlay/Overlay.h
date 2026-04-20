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
        void Clear()
        {
            m_objects.clear();
        }

        void Add(std::unique_ptr<Object> obj)
        {
            m_objects.push_back(std::move(obj));
        }

        void ForEach(std::function<void(const Object&)> fn) const
        {
            for (auto& obj : m_objects)
                fn(*obj);
        }

        void ToVertices(std::vector<Vertex_P3_C4>& out) const
        { 
            for (const auto& obj : m_objects)
            {
                if (!obj) continue;

                if (obj->IsKindOf<LineEntity>())
                {
                    const auto& line = static_cast<const LineEntity&>(*obj);
                    const auto& color = line.GetAttr().Color;

                    const auto& start = line.GetLine().Start;
                    const auto& end = line.GetLine().End;

                    out.push_back({ start, color });
                    out.push_back({ end,   color });
                }
            }
        }

    private:
        std::vector<std::unique_ptr<Object>> m_objects;
    };
}