#pragma once  
#include <DirectXMath.h>
#include "Core/Object/Object.hpp"
#include "Core/Object/RuntimeType.hpp"
#include "Core/GeomKernel/Circle.hpp" 
#include "Entity.hpp"
using namespace DirectX;

namespace MiniCAD
{
    class CircleEntity : public Entity
    {
    public:
        CircleEntity(ObjectID id, const XMFLOAT3& center, float radius)
            : Entity(id)
            , m_circle(center, radius)
        {
        }

        const Circle& GetCircle() const { return m_circle; }
        AABB GetBoundingBox() const override { return m_circle.GetBounds(); }
        DECLARE_RUNTIME_TYPE(CircleEntity, Entity)
    private:
        Circle m_circle;
    };
     

}