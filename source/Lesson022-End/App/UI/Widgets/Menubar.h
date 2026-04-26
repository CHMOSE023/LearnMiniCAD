#pragma once
#include <string>
#include "ImGuiWidgetBase.h" 
namespace MiniCAD
{
    class Menubar : public ImGuiWidgetBase
    {
    public:
        virtual void        OnRender(Document& editor) override;
        virtual const char* GetName() const  override;
    };
}