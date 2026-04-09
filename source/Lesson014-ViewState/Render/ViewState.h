#pragma once 
#include "Core/Object/Object.hpp"
#include <unordered_set>
namespace MiniCAD
{
    /// <summary>
	/// View 状态桥梁,Editor 维护交互状态（Selection/Hovered），Viewport 维护渲染开关（ShowGrid/ShowGizmo），它们通过 ViewState 进行桥接，解耦彼此。
    /// </summary>
    struct ViewState
    {   
        using ObjectID = Object::ObjectID;

        // 交互结果
        const std::unordered_set<ObjectID>* Selection = nullptr;
        const std::unordered_set<ObjectID>* Hovered   = nullptr;

        // 渲染开关
        bool ShowGrid  = true;
        bool ShowGizmo = true;
    };
}
