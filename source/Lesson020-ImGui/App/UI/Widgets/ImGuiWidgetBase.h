#pragma once
#include <string>

namespace MiniCAD
{
    class ImGuiWidgetBase
    {
    public:
        virtual ~ImGuiWidgetBase() = default;

        virtual void OnRender() = 0;

        virtual const char* GetName() const = 0;

        bool Visible = true;
    };
}