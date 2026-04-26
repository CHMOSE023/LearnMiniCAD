#pragma once
#include "ImGuiWidgetBase.h" 
#include "PropertySubPanelBase.h"
#include <string> 
#include <unordered_map>
#include <typeindex>
#include <memory>
namespace MiniCAD
{
    class PropertyPanel : public  ImGuiWidgetBase
    {
    public:
        PropertyPanel();
        virtual void        OnRender(Document& document) override;
        virtual const char* GetName() const  override; 
    private:
        std::unordered_map<std::type_index, std::unique_ptr<PropertySubPanelBase>> m_subPanels;
    };
}