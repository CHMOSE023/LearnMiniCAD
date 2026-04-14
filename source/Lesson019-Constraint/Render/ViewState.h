#pragma once 
#include "Core/Object/Object.hpp"
#include <unordered_set>
namespace MiniCAD
{
    struct GripDraw
    {
        enum class Type : uint8_t { Start, Mid, End };
        DirectX::XMFLOAT2 Pos;
        Type              GripType;
        bool              Hovered;
    };

    struct SnapDraw
    {
        enum class Type : uint8_t { None, Endpoint, Midpoint, Nearest, Grid };
        Type              SnapType = Type::None;
        DirectX::XMFLOAT2 Pos= {};
        bool              IsValid() const { return SnapType != Type::None; }
    };

    /// <summary>
	/// View 状态桥梁,Editor 维护交互状态（Selection/Hovered），Viewport 维护渲染开关（ShowGrid/ShowGizmo），它们通过 ViewState 进行桥接，解耦彼此。
    /// </summary>
    struct ViewState
    {   
        using ObjectID = Object::ObjectID;

        // 交互结果
        const std::unordered_set<ObjectID>* Selection = nullptr;
        const std::unordered_set<ObjectID>* Hovered   = nullptr;

        std::vector<GripDraw> Grips;

        // 渲染开关
        bool ShowGrid  = true;
        bool ShowGizmo = true;

        bool CrossBox  = true;  

		float MouseX = 0.0f; // 可选：鼠标位置（屏幕坐标）
		float MouseY = 0.0f;  
           
        // 是否显示框选矩形（由 Editor 控制）

        bool  BoxSelected = true;
        float BoxPressX   = 0.0f;
        float BoxPressY   = 0.0f;

		// Snap状态
        SnapDraw Snap;
         
    };
}
