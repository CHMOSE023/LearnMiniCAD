#pragma once
#include <string>
#include "ImGuiWidgetBase.h"  
namespace MiniCAD
{
    class StatusBar : public  ImGuiWidgetBase
    {
    public:
        virtual void        OnRender( Document& document) override;
        virtual const char* GetName() const  override; 
    };
}