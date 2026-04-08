#pragma once
#include <vector>
namespace MiniCAD
{
    class LineEntity;
    class CircleEntity;

    // Scene 数据访问接口（解耦 Scene）
    class ISceneReader
    {
	public:
        virtual const std::vector<LineEntity>&   GetLines()   const = 0;
        virtual const std::vector<CircleEntity>& GetCircles() const = 0;
        virtual ~ISceneReader() = default;
    };

}