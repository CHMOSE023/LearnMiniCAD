#pragma once   
#include "App/Input/IInputHandler.h" 
#include "App/Input/InputEvent.h" 
#include "App/Editor/Picking/Picking.h"
#include "App/Scene/Scene.h" 
#include "App/CommandStack/CommandStack.h" 
#include "Render/Viewport/Viewport.h"
#include <unordered_set> 
#include <DirectXMath.h>
#include <memory>
#include "App/Editor/Tools/ITool.h"
#include "Render/ViewState.h"
#include "Grip/GripManager.h"
#include "Grip/GripEditor.h"
#include "Snap/SnapEngine.h"
namespace MiniCAD
{ 
    class Editor : public IInputHandler
    {
    public:
        Editor(Scene* scene, CommandStack* cmdStack); 
        bool OnInput(const InputEvent& e) override;  
        void OnResize(float width, float height);
        ViewState BuildViewState() const;
        const std::unordered_set<Object::ObjectID>& GetSelection();
        const std::unordered_set<Object::ObjectID>& GetHovered(); 
        Scene* GetScene() const { return m_scene; }
        bool TryGetAnchor(DirectX::XMFLOAT3& out) const;
        bool IsOrthoEnabled() const { return m_orthoEnabled; }
    private:
        void OnMouseButtonDown(const InputEvent& e);
        void OnMouseButtonUp  (const InputEvent& e);
        void OnKeyDown        (const InputEvent& e);
        void OnKeyUp          (const InputEvent& e);
        void OnMouseMove      (const InputEvent& e);   
        void OnMouseWheel     (const InputEvent& e);
          
    private:
        void DeleteSelected();
        void OnSelectionChanged();   // 新增：选集变化时统一处理
        void UpdateSnap(const InputEvent& e); 
        bool ShouldSnap() const;                           // 是否允许捕获
        InputEvent  InjectSnap(const InputEvent& e);       // 注入捕获事件
        InputEvent  ApplyConstraints(const InputEvent& e); // 约束事件
    private: 
        Scene*        m_scene        = nullptr;      
        CommandStack* m_cmdStack     = nullptr; 
        Viewport*     m_view         = nullptr; 
        bool          m_showGrid     = true;     // 网格
        bool          m_showGizmo    = true;     // 夹点
        bool          m_orthoEnabled = true;     // 正交
        std::unique_ptr<ITool>   m_tool;

        Picking       m_picking;
        GripManager   m_gripManager;   // 新增
        GripEditor    m_gripEditor;    // 新增
		float 	      m_mouseX = 0.f;  // 鼠标位置（屏幕坐标）
        float         m_mouseY = 0.f; 

        SnapEngine    m_snap;
        SnapResult    m_currentSnap;
         
    };
}