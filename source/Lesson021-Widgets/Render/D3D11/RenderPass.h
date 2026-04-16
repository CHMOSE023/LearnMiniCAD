#pragma once  
#include "pch.h"
namespace MiniCAD
{
    /// <summary>
    /// 一个逻辑渲染阶段的状态切换,控制内容 VP|Depth|Shader
    /// </summary>
    struct RenderPassDesc
    {
        XMMATRIX  ViewProj;
        bool      Depth;
    };
}
