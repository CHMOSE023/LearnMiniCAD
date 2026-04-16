#pragma once
#include <string> 
#include "ImGuiWidgetBase.h" 
namespace MiniCAD
{
    class PropertyPanel : public  ImGuiWidgetBase
    {
    public:
        virtual void        OnRender(Document& document) override;
        virtual const char* GetName() const  override; 
    private:
        // 生成直线属性面板
        // 生成圆属性面板
        // 生成块属性面板
    };
}