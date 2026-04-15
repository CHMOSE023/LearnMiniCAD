#pragma once
#include "App/Input/InputEvent.h"

namespace MiniCAD
{
    class Editor;

    class IInputConstraint
    {
    public:
        virtual ~IInputConstraint() = default;

        // 是否启用（比如 F8 开关）
        virtual bool IsEnabled(const Editor& editor) const = 0;

        // 对输入进行修改
        virtual InputEvent Apply(const InputEvent& e, const Editor& editor) = 0;
    };
}