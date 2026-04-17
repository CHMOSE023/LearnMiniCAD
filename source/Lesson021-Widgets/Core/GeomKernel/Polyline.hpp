#pragma once
#include <DirectXMath.h>
#include <vector>
#include "AABB.hpp"
#include "Line.hpp"   // 复用 Line 的 ClosestPoint / DistanceToPoint
using namespace DirectX;

namespace MiniCAD
{
    struct Polyline
    {
        std::vector<XMFLOAT3> Points;
        bool Closed = false;

        bool IsValid() const { return Points.size() >= 2; }

        int SegmentCount() const
        {
            if (Points.size() < 2) return 0;
            return (int)Points.size() - 1 + (Closed ? 1 : 0);
        }

        // 取第 i 段线段（自动处理闭合的最后一段）
        Line GetSegment(int i) const
        {
            int n = (int)Points.size();
            return Line(Points[i % n], Points[(i + 1) % n]);
        }

        // =============================
        // 几何计算（捕捉用）
        // =============================
        XMFLOAT3 ClosestPoint(const XMFLOAT3& p) const
        {
            XMFLOAT3 best = Points[0];
            float    bestDist = FLT_MAX;
            for (int i = 0; i < SegmentCount(); ++i)
            {
                XMFLOAT3 cp = GetSegment(i).ClosestPoint(p);
                XMVECTOR vcp = XMLoadFloat3(&cp);
                XMVECTOR vpt = XMLoadFloat3(&p);
                float    dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(vpt, vcp)));
                if (dist < bestDist) { bestDist = dist; best = cp; }
            }
            return best;
        }

        float DistanceToPoint(const XMFLOAT3& p) const
        {
            float best = FLT_MAX;
            for (int i = 0; i < SegmentCount(); ++i)
                best = std::min(best, GetSegment(i).DistanceToPoint(p));
            return best;
        }

        // =============================
        // 包围盒
        // =============================
        AABB GetBounds() const
        {
            AABB box;
            box.Min = box.Max = Points[0];
            for (const auto& pt : Points)
            {
                box.Min = { std::min(box.Min.x, pt.x),
                            std::min(box.Min.y, pt.y),
                            std::min(box.Min.z, pt.z) };
                box.Max = { std::max(box.Max.x, pt.x),
                            std::max(box.Max.y, pt.y),
                            std::max(box.Max.z, pt.z) };
            }
            return box;
        }
    };
}
