#pragma once
#include <string>
#include "ImGuiWidgetBase.h" 
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

namespace MiniCAD
{
    class ToolBar : public  ImGuiWidgetBase
    {
    public:
        ToolBar();

        virtual void        OnRender(Document& document) override;
        virtual const char* GetName() const  override; 
         
    private:
        void RenderAxisWindow(Document& document);

        struct Axis
        {
            float       pos;
            float       realPos;
            bool        vertical;
            bool        visible;
            std::string name;
        };

        // 轴网状态提升为成员变量（原来是 static）
        bool              m_showAxisWindow = false;
        std::vector<Axis> m_axes;
        int               m_axisCounter = 1;
        int               m_addVertical = 1;
        int               m_dragging    = -1;
        float             m_axisExtend  = 1.5f;
        XMFLOAT2          m_gridOrigin  = { 0.0f, 0.0f };
    };
}