#pragma once
#include "pch.h"
#include <memory> 
#include "ImGuiLayer.h"
#include "Widgets/ImGuiWidgetBase.h"
namespace MiniCAD
{
    class Document;
    class UIManager
    {
    public:
        bool Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void BeginFrame();
        void Render    (Document& doc);
        void EndFrame  ();

        ImGuiWidgetBase* FindWidget(const std::string& id);
    private:
        void DrawDockSpace(Document& doc);

    private:
        std::unique_ptr<ImGuiLayer> m_imgui;

        std::vector<std::unique_ptr<ImGuiWidgetBase>> m_widgets; 
    };
}