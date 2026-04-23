#pragma once
#include "SnapResult.h"
#include "App/Scene/Scene.h"
#include "Render/Viewport/Camera.h"

namespace MiniCAD
{
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
        SnapResult Query(const DirectX::XMFLOAT2& screenPt, const Scene& scene, const Camera& cam) const;

    private:
        SnapResult TryEndpoint(const DirectX::XMFLOAT2& sp, const Scene&, const Camera&) const;
        SnapResult TryMidpoint(const DirectX::XMFLOAT2& sp, const Scene&, const Camera&) const;
        SnapResult TryNearest (const DirectX::XMFLOAT2& sp, const Scene&, const Camera&) const;
        SnapResult TryGrid    (const DirectX::XMFLOAT2& sp, const Camera&)               const;
    };
}
