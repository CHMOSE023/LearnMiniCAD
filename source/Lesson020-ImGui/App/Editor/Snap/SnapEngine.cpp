#include "SnapEngine.h"
#include "Core/Entity/LineEntity.hpp"
#include <cmath>
#include <algorithm>

using namespace DirectX;

namespace MiniCAD
{
    static float Dist2D(const XMFLOAT2& a, const XMFLOAT2& b)
    {
        return std::hypot(a.x - b.x, a.y - b.y);
    }

    static XMFLOAT3 ClosestPointOnSegment(const XMFLOAT3& p, const XMFLOAT3& a, const XMFLOAT3& b)
    {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float lenSq = dx * dx + dy * dy;

        if (lenSq < 1e-8f)
            return a;

        float t = std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / lenSq, 0.f, 1.f);

        return { a.x + t * dx, a.y + t * dy, 0.f };
    }

    // ───────────── 主入口 ─────────────

    SnapResult SnapEngine::Query(const SnapContext& ctx) const
    {
        if (EnableEndpoint)
        {
            auto r = TryEndpoint(ctx);
            if (r.IsValid()) return r;
        }

        if (EnableMidpoint)
        {
            auto r = TryMidpoint(ctx);
            if (r.IsValid()) return r;
        }

        if (EnableNearest)
        {
            auto r = TryNearest(ctx);
            if (r.IsValid()) return r;
        }

        if (EnableGrid)
            return TryGrid(ctx);

        return {};
    }

    // ───────────── Endpoint ─────────────

    SnapResult SnapEngine::TryEndpoint(const SnapContext& ctx) const
    {
        SnapResult best;
        float bestDist = FLT_MAX;

        ctx.scene->ForEachObject([&](const Object& obj)
        {
            if (ctx.exclude.contains(obj.GetID()))
                return;

            if (!obj.IsKindOf<LineEntity>())
                return;

            auto* line = static_cast<const LineEntity*>(&obj); 

            for (const XMFLOAT3& wp : { line->GetLine().Start, line->GetLine().End })
            {
                float d = Dist2D(ctx.screenPt, ctx.cam->WorldToScreen(wp));

                if (d < SnapRadiusPx && d < bestDist)
                {
                    bestDist = d;
                    best = { SnapResult::Type::Endpoint, wp, obj.GetID() };
                }
            }
        });

        return best;
    }

    // ───────────── Midpoint ─────────────

    SnapResult SnapEngine::TryMidpoint(const SnapContext& ctx) const
    {
        SnapResult best;
        float bestDist = FLT_MAX;

        ctx.scene->ForEachObject([&](const Object& obj)
            {
                if (ctx.exclude.contains(obj.GetID()))
                    return;
                 
                if (!obj.IsKindOf<LineEntity>())
                    return;

                auto* line = static_cast<const LineEntity*>(&obj);

                auto& L = line->GetLine();

                XMFLOAT3 mid =
                {
                    (L.Start.x + L.End.x) * 0.5f,
                    (L.Start.y + L.End.y) * 0.5f,
                    0.f
                };

                float d = Dist2D(ctx.screenPt, ctx.cam->WorldToScreen(mid));

                if (d < SnapRadiusPx && d < bestDist)
                {
                    bestDist = d;
                    best = { SnapResult::Type::Midpoint, mid, obj.GetID() };
                }
            });

        return best;
    }

    // ───────────── Nearest ─────────────

    SnapResult SnapEngine::TryNearest(const SnapContext& ctx) const
    {
        SnapResult best;
        float bestDist = FLT_MAX;

        XMFLOAT3 worldMouse = ctx.cam->ScreenToWorld(ctx.screenPt.x, ctx.screenPt.y);

        ctx.scene->ForEachObject([&](const Object& obj)
            {
                if (ctx.exclude.contains(obj.GetID()))
                    return;

                if (!obj.IsKindOf<LineEntity>())
                    return;

                auto* line = static_cast<const LineEntity*>(&obj);

                auto& L = line->GetLine();

                XMFLOAT3 closest =  ClosestPointOnSegment(worldMouse, L.Start, L.End);

                float d = Dist2D(ctx.screenPt, ctx.cam->WorldToScreen(closest));

                if (d < SnapRadiusPx && d < bestDist)
                {
                    bestDist = d;
                    best = { SnapResult::Type::Nearest, closest, obj.GetID() };
                }
            });

        return best;
    }

    // ───────────── Grid ─────────────

    SnapResult SnapEngine::TryGrid(const SnapContext& ctx) const
    {
        XMFLOAT3 w = ctx.cam->ScreenToWorld(ctx.screenPt.x, ctx.screenPt.y);

        return
        {
            SnapResult::Type::Grid,
            {
                std::round(w.x / GridSize) * GridSize,
                std::round(w.y / GridSize) * GridSize,
                0.f
            },
            Object::InvalidID
        };
    }
}