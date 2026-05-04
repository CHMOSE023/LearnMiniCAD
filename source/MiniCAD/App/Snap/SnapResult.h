#pragma once
#include <DirectXMath.h>
#include "Core/Object/Object.hpp"

namespace MiniCAD
{
    struct SnapResult
    {
        enum class Type : uint8_t { None, Endpoint, Midpoint, Nearest, Grid };
        Type              SnapType = Type::None;
        DirectX::XMFLOAT3 WorldPos = {};
        Object::ObjectID  SourceID = Object::InvalidID;

        bool IsValid() const { return SnapType != Type::None; }
    };
}
