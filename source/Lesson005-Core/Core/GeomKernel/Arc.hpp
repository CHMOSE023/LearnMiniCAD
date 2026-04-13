#pragma once
#include <DirectXMath.h>
#include <cmath>
#include <vector>
#include "AABB.hpp"
using namespace DirectX;

namespace MiniCAD
{
    struct Arc
    {
        XMFLOAT3 Center;
        float    Radius;
        float    StartAngle;  // 弧度，0 = 右（+X 方向）
        float    EndAngle;    // 逆时针为正

        // =============================
        // 工具
        // =============================
        bool IsValid() const { return Radius > EPSILON; }

        // 角度是否在弧的范围内
        bool ContainsAngle(float angle) const
        {
            // 归一化到 [StartAngle, StartAngle + span]
            float span = EndAngle - StartAngle;
            float t = std::fmod(angle - StartAngle, XM_2PI);
            if (t < 0) t += XM_2PI;
            return t <= span + EPSILON;
        }

        // 圆周上某角度的点
        XMFLOAT3 PointAtAngle(float angle) const
        {
            return { Center.x + Radius * std::cos(angle),   Center.y + Radius * std::sin(angle),  Center.z };
        }

        // =============================
        // 特征点
        // =============================
        XMFLOAT3 StartPoint() const { return PointAtAngle(StartAngle); }
        XMFLOAT3 EndPoint()   const { return PointAtAngle(EndAngle); }
        XMFLOAT3 Midpoint()   const { return PointAtAngle((StartAngle + EndAngle) * 0.5f); }

        // =============================
        // 几何计算（捕捉用）
        // =============================
        XMFLOAT3 ClosestPoint(const XMFLOAT3& p) const
        {
            XMVECTOR c = XMLoadFloat3(&Center);
            XMVECTOR pt = XMLoadFloat3(&p);
            XMVECTOR d = XMVectorSubtract(pt, c);

            // p 到圆心的角度
            float angle = std::atan2(XMVectorGetY(d), XMVectorGetX(d));

            if (ContainsAngle(angle))
            {
                // 角度在弧范围内，直接投影到圆周
                return PointAtAngle(angle);
            }
            else
            {
                // 角度在弧外，最近点是离 p 更近的那个端点
                XMFLOAT3 s  = StartPoint();
                XMFLOAT3 e  = EndPoint();
                XMVECTOR vs = XMLoadFloat3(&s);
                XMVECTOR ve = XMLoadFloat3(&e);
                float ds = XMVectorGetX(XMVector3Length(XMVectorSubtract(pt, vs)));
                float de = XMVectorGetX(XMVector3Length(XMVectorSubtract(pt, ve)));
                return ds < de ? s : e;
            }
        }

        float DistanceToPoint(const XMFLOAT3& p) const
        {
            XMFLOAT3 cp = ClosestPoint(p);
            XMVECTOR vcp = XMLoadFloat3(&cp);
            XMVECTOR vpt = XMLoadFloat3(&p);
            return XMVectorGetX(XMVector3Length(XMVectorSubtract(vpt, vcp)));
        }

        // =============================
        // 包围盒（需要考虑穿越 0/90/180/270 度的情况）
        // =============================
        AABB GetBounds() const
        {
            float xmin = std::min(StartPoint().x, EndPoint().x);
            float xmax = std::max(StartPoint().x, EndPoint().x);
            float ymin = std::min(StartPoint().y, EndPoint().y);
            float ymax = std::max(StartPoint().y, EndPoint().y);

            // 检查弧是否穿过四个轴方向极值点
            float axes[4] = { 0, XM_PIDIV2, XM_PI, 3 * XM_PIDIV2 };
            for (float a : axes)
            {
                if (ContainsAngle(a))
                {
                    XMFLOAT3 pt = PointAtAngle(a);
                    xmin = std::min(xmin, pt.x);
                    xmax = std::max(xmax, pt.x);
                    ymin = std::min(ymin, pt.y);
                    ymax = std::max(ymax, pt.y);
                }
            }
            return { { xmin, ymin, Center.z }, { xmax, ymax, Center.z } };
        }

        // =============================
        // 离散化
        // =============================
        std::vector<XMFLOAT3> Tessellate(int segments = 32) const
        {
            std::vector<XMFLOAT3> pts;
            pts.reserve(segments + 1);
            float span = EndAngle - StartAngle;
            for (int i = 0; i <= segments; ++i)
            {
                float t = static_cast<float>(i) / segments;
                pts.push_back(PointAtAngle(StartAngle + span * t));
            }
            return pts;
        }

        static constexpr float EPSILON = 1e-6f;
    };
}
