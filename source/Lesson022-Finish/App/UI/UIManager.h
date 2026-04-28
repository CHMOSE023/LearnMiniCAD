#pragma once
#include "pch.h"
#include <memory> 
#include "ImGuiLayer.h"
#include "Widgets/ImGuiWidgetBase.h"
namespace MiniCAD
{
    class Document;
    class DocumentManager;
    class UIManager
    {
    public:
        bool Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void BeginFrame();
        void Render    (DocumentManager& dm);
        void EndFrame  ();

        void  DrawDocumentTabs(DocumentManager& dm);

        ImGuiWidgetBase* FindWidget(const std::string& id);
    private:
        void DrawDockSpace();

    private:
        std::unique_ptr<ImGuiLayer> m_imgui;

        std::vector<std::unique_ptr<ImGuiWidgetBase>> m_widgets; 
    };
}