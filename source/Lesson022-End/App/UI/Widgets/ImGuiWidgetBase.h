#pragma once
#include <string>

namespace MiniCAD
{
    class Document;
    class ImGuiWidgetBase
    {
    public:
        virtual ~ImGuiWidgetBase() = default;

        virtual void        OnRender(Document& document) = 0;
        virtual const char* GetName() const = 0;
    };
}