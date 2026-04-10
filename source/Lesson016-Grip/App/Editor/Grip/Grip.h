#pragma once 
#include "Core/Object/Object.hpp"
#include <DirectXMath.h>

namespace MiniCAD
{
    struct Grip
    {
        enum class Type : uint8_t { Start, Mid, End };

        Object::ObjectID  OwnerID;   // 属于哪条线
        Type              Type;
        DirectX::XMFLOAT3 WorldPos; // 世界坐标（用于捕捉计算）
    };
}