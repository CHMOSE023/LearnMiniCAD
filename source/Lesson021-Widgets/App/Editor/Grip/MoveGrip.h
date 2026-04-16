#pragma once 
#include <DirectXMath.h> 
#include "App/Editor/Grip/Grip.h"
/// <summary>
/// 夹点操作
/// </summary>
namespace MiniCAD 
{ 
    struct LineSegment // 线段
    {
        DirectX::XMFLOAT3 Start;
        DirectX::XMFLOAT3 End;
    };

    LineSegment MoveGrip(const LineSegment& seg, Grip::Type type, const DirectX::XMFLOAT3& p); 

}