#pragma once 
#include "App/Editor//Editor.h"
#include "ImGuiLayer.h"
#include "Widgets/ImGuiWidgetBase.h"

namespace MiniCAD
{  
    class UIManager
    {
    public:
        bool Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void BeginFrame();
        void Render(  Editor& editor)  ;
        void EndFrame();

    private:
        void DrawDockSpace(  Editor& editor)  ;
        void DrawPanels   (  Editor& editor)  ;


    private:
        std::unique_ptr<ImGuiLayer> m_imgui;
        std::vector<std::unique_ptr<ImGuiWidgetBase>> m_widgets;
    };
}