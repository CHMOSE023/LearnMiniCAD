#pragma once
#include "SnapResult.h"
#include "App/Scene/Scene.h"
#include "Render/Viewport/Camera.h"
#include <unordered_set>

namespace MiniCAD
{
    struct SnapContext
    {
        DirectX::XMFLOAT2 screenPt;   // 鼠标屏幕坐标
        DirectX::XMFLOAT3 rayOrigin;  // 可选（后续扩展）
        DirectX::XMFLOAT3 rayDir;

        Scene* scene = nullptr;
        Camera* cam = nullptr;

        std::unordered_set<Object::ObjectID> exclude;
    };

    class SnapEngine
    {
    public:
        // ─── 开关 ───────────────────────────────
        bool  EnableEndpoint = true;
        bool  EnableMidpoint = true;
        bool  EnableNearest  = true;
        bool  EnableGrid     = false;
        float GridSize       = 1.0f;   // 世界单位
        float SnapRadiusPx   = 12.f;   // 屏幕像素捕捉半径

        // ─── 主接口 ─────────────────────────────
        // screenPt: 鼠标屏幕坐标
        // 返回优先级最高的捕捉结果，无捕捉时 type == None（Grid 永远命中）
        SnapResult Query(const SnapContext& ctx) const;

    private:
        SnapResult TryEndpoint(const SnapContext& ctx) const;
        SnapResult TryMidpoint(const SnapContext& ctx) const;
        SnapResult TryNearest (const SnapContext& ctx) const;
        SnapResult TryGrid    (const SnapContext& ctx)         const;
    };
}