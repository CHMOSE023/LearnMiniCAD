#pragma once
#include "IInputConstraint.h"
#include <DirectXMath.h>

namespace MiniCAD
{
    using namespace DirectX;

    class OrthoConstraint : public IInputConstraint
    {
    public:
        bool IsEnabled(const Editor& editor) const override;

        InputEvent Apply(const InputEvent& e, const Editor& editor) override;

    private:
        // 核心正交计算
        XMFLOAT2 ApplyOrtho(const XMFLOAT2& base, const XMFLOAT2& input) const;
    };
}