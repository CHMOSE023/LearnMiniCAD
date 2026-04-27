#pragma once
#include "App/UI/Widgets/ImGuiWidgetBase.h"
#include <cstdint>

namespace MiniCAD
{
    class LayerManager;
    class Document;

    class LayerManagerWidget : public ImGuiWidgetBase
    {
    public:
        LayerManagerWidget();

        void        OnRender(Document& document) override;
        const char* GetName() const override { return "图层管理"; }

    private:
        // 颜色选择器：记录当前正在编辑颜色的图层 ID，kInvalidID 表示"无"
        static constexpr uint32_t kInvalidID = 0xFFFF'FFFFu;  

        bool     m_colorPickerShouldOpen = false;  //  新增
        uint32_t m_colorPickerLayerID  = kInvalidID;

        // 新建图层对话框
        char m_newLayerName[64] = "New Layer"; 

        void RenderToolbar  (LayerManager& mgr);
        void RenderLayerList(LayerManager& mgr);
        void RenderAddDialog(LayerManager& mgr);
    };
}