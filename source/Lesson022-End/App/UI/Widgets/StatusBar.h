#pragma once
#include <string>
#include "ImGuiWidgetBase.h"  
#include <DirectXMath.h>
namespace MiniCAD
{
    class StatusBar : public  ImGuiWidgetBase
    {
    public: 
        StatusBar(); 

        virtual void        OnRender( Document& document) override;
        virtual const char* GetName() const  override; 
    
    private:
        DirectX::XMFLOAT3 m_worldFrozen {};

        bool     m_worldActive = true; 
        
    };
}