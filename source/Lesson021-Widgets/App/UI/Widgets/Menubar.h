#pragma once
#include <string>
#include "ImGuiWidgetBase.h" 
namespace MiniCAD
{
    class UIManager; // 前向声明
    class Menubar : public ImGuiWidgetBase
    {
    public:
        explicit Menubar(UIManager* mgr) : m_ui(mgr) {}

        virtual void        OnRender(Document& editor) override;
        virtual const char* GetName() const  override;

    private:
        UIManager* m_ui = nullptr;
    };
}