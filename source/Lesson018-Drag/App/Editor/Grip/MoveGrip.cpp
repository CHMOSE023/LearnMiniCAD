#include "App/Editor/Grip/MoveGrip.h"
using namespace DirectX;

namespace MiniCAD 
{  
    LineSegment MoveGrip(const LineSegment& seg, Grip::Type type, const XMFLOAT3& p)
    {
        LineSegment out = seg; // copy 一份（关键）

        switch (type)
        {
        case Grip::Type::Start:
            out.Start = p;
            break;

        case Grip::Type::End:
            out.End = p;
            break;

        case Grip::Type::Mid:
        {
            XMFLOAT3 mid{ (seg.Start.x + seg.End.x) * 0.5f,  (seg.Start.y + seg.End.y) * 0.5f,  0.0f };

            float dx = p.x - mid.x;
            float dy = p.y - mid.y;

            out.Start.x += dx;
            out.Start.y += dy;

            out.End.x += dx;
            out.End.y += dy;
            break;
        }
        }

        return out;
    }
}