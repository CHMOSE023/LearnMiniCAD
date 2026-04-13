#pragma once
#include <DirectXMath.h>
#include <cmath>
#include <vector>
#include "AABB.hpp"
using namespace DirectX;

namespace MiniCAD
{
    struct Circle
    {
        XMFLOAT3 Center;
        float    Radius;

        // =============================
        // 基础属性
        // =============================
        bool IsValid() const { return Radius > EPSILON; }

        // =============================
        // 几何计算（捕捉用）
        // =============================

        // 圆上离 p 最近的点（垂足投影到圆周）
        XMFLOAT3 ClosestPoint(const XMFLOAT3& p) const
        {
            XMVECTOR c  = XMLoadFloat3(&Center);
            XMVECTOR pt = XMLoadFloat3(&p);
            XMVECTOR d  = XMVectorSubtract(pt, c);
            float    len = XMVectorGetX(XMVector3Length(d));
            if (len < EPSILON)
            {
                // p 在圆心，随便返回一个点（0度方向）
                return { Center.x + Radius, Center.y, Center.z };
            }
            XMVECTOR closest = XMVectorAdd(c, XMVectorScale(XMVector3Normalize(d), Radius));
            XMFLOAT3 result;
            XMStoreFloat3(&result, closest);
            return result;
        }

        // p 到圆的最短距离（到圆周，不是圆心）
        float DistanceToPoint(const XMFLOAT3& p) const
        {
            XMVECTOR c  = XMLoadFloat3(&Center);
            XMVECTOR pt = XMLoadFloat3(&p);
            float distToCenter = XMVectorGetX(XMVector3Length(XMVectorSubtract(pt, c)));
            return std::abs(distToCenter - Radius);
        }

        // =============================
        // 特征点（端点捕捉）
        // =============================
        XMFLOAT3 QuadrantPoint(int i) const  // i = 0/1/2/3 → 右/上/左/下
        {
            float angle = XM_PIDIV2 * i;
            return { Center.x + Radius * std::cos(angle),  Center.y + Radius * std::sin(angle),  Center.z };
        }

        // =============================
        // 包围盒
        // =============================
        AABB GetBounds() const
        {
            return { { Center.x - Radius, Center.y - Radius, Center.z }, { Center.x + Radius, Center.y + Radius, Center.z } };
        }

        // =============================
        // 离散化（Render 层用）
        // =============================
        std::vector<XMFLOAT3> Tessellate(int segments = 64) const
        {
            std::vector<XMFLOAT3> pts;
            pts.reserve(segments);
            const float step = XM_2PI / segments;
            for (int i = 0; i < segments; ++i)
            {
                float a = step * i;
                pts.push_back({ Center.x + Radius * std::cos(a),  Center.y + Radius * std::sin(a),   Center.z });
            }
            return pts;  // 调用方 push_back(pts[0]) 闭合
        }

        static constexpr float EPSILON = 1e-6f;
    };
}
