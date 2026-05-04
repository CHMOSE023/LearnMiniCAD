#pragma once
#include "pch.h"
#include "ImGuiLayer.h" 
#include <memory> 
#include <imgui.h> 
namespace MiniCAD
{ 
    class DocumentManager;

    enum class Tool : int
    {
        Select = 0,
        Line,
        Circle,
        Rectangle,
        Arc,
        Pan,
        Zoom,
        COUNT
    };

    // =========================================================
    // DocImageState
    // 每帧由 DrawDocumentTabs 写入，供 EventProc 读取
    // =========================================================
    struct DocImageState
    {  
        // --- 图像信息 ---
        ImVec2 Size;         // 显示尺寸（窗口中）
        ImVec2 Min;          // 屏幕坐标：左上角
        ImVec2 Max;          // 屏幕坐标：右下角

        // --- 输入 ---
        ImVec2 MousePos;    // 鼠标屏幕坐标
        ImVec2 Local;       // 相对图像坐标 

        float  Wheel = 0;   // 滚轮
        ImVec2 Delta;       // 偏移量
        bool   Hovered = false; // 本帧鼠标是否悬停在 Image 上
    };


    class UIManager
    {
    public:
        bool Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
        void Shutdown();

        void BeginFrame();
        void EndFrame  ();

        void Render (DocumentManager& dm); 

        float GetCaptionButtonsX() const { return m_captionButtonsScreenX; };
         
        const DocImageState& GetDocImageState() const { return m_docImageState; }
        Tool                 GetActiveTool()    const { return m_activeTool; }
    private: 
        void DrawMenubar     (DocumentManager& dm);    
        void DrawToolbar     (DocumentManager& dm);
        void DrawDocumentTabs(DocumentManager& dm);
        void DrawStatusBar   (DocumentManager& dm);


    private:
        std::unique_ptr<ImGuiLayer> m_imgui;   
        HWND                        m_hwnd = nullptr;
        float                       m_captionButtonsScreenX=0.f;

        DocImageState m_docImageState{};
        ImVec2        m_lastLocal = ImVec2(0, 0);
        Tool m_activeTool = Tool::Select;
    };
}