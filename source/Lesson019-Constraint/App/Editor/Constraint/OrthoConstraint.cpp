#include "OrthoConstraint.h"
#include "App/Editor/Editor.h"
#include "Render/Viewport/Camera.h"

#include <cmath>

namespace MiniCAD
{
    using namespace DirectX;

    // ─────────────────────────────────────────────
    // 是否启用（F8）
    // ─────────────────────────────────────────────
    bool OrthoConstraint::IsEnabled(const Editor& editor) const
    {
        return editor.IsOrthoEnabled();
    }

    // ─────────────────────────────────────────────
    // 核心：应用约束
    // ─────────────────────────────────────────────
    InputEvent OrthoConstraint::Apply(const InputEvent& e, const Editor& editor)
    {
        // 只处理鼠标相关事件
        if (!(e.Type == InputEventType::MouseMove ||
              e.Type == InputEventType::MouseButtonDown ||
              e.Type == InputEventType::MouseButtonUp))
        {
            return e;
        }

        // 获取锚点（LineTool起点 / Grip拖拽起点）
        XMFLOAT3 base;
        if (!editor.TryGetAnchor(*(XMFLOAT3*)&base)) // 兼容你现有 Vec2
            return e;

        // 当前输入点（优先 Snap）
        XMFLOAT2 input;

        if (e.HasSnap)
        {
            input = { e.SnapWorld.x, e.SnapWorld.y };
        }
        else
        {
            auto* cam = editor.GetScene()->GetCamera();
            auto world = cam->ScreenToWorld((float)e.MouseX, (float)e.MouseY);
            input = { world.x, world.y };
        }

        // 应用正交
        XMFLOAT2 result = ApplyOrtho({ base.x,base.y }, input);

        // 输出新的事件（覆盖 Snap）
        InputEvent out = e;
        out.HasSnap = true;
        out.SnapWorld = { result.x, result.y ,0 };

        return out;
    }

    // ─────────────────────────────────────────────
    // 正交计算（核心算法）
    // ─────────────────────────────────────────────
    XMFLOAT2 OrthoConstraint::ApplyOrtho(const XMFLOAT2& base, const XMFLOAT2& input) const
    {
        float dx = input.x - base.x;
        float dy = input.y - base.y;

        if (std::fabs(dx) > std::fabs(dy))
        {
            // 水平锁定
            return { input.x, base.y };
        }
        else
        {
            // 垂直锁定
            return { base.x, input.y };
        }
    }
}