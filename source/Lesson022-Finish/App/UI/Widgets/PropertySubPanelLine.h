#pragma once
#include "PropertySubPanelBase.h"
#include "pch.h" 
namespace MiniCAD
{
    using namespace DirectX;

    class PropertySubPanelLine : public PropertySubPanelBase
    {  
        struct ColorItem
        {
            const char* name;
            XMFLOAT4    value;
        };

        static constexpr ColorItem colors[] =
        {
            { "白色", {1,1,1,1} },
            { "黑色", {0,0,0,1} },
            { "红色", {1,0,0,1} },
            { "绿色", {0,1,0,1} },
            { "蓝色", {0,0,1,1} },
            { "黄色", {1,1,0,1} },
            { "灰色", {0.5f,0.5f,0.5f,1} }
        };

    public:
        virtual void OnRender(Object* entity, Document& document) override;

    private:
        float m_labelWidth = 90.0f;
        float m_inputWidth = 180.0f;
    };
}
