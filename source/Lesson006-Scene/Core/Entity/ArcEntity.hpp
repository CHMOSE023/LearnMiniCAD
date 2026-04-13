#pragma once  
#include <DirectXMath.h>
#include "Core/Object/Object.hpp"
#include "Core/Object/RuntimeType.hpp"
#include "Core/GeomKernel/Arc.hpp" 
#include "Entity.hpp"
using namespace DirectX;

namespace MiniCAD
{  
    class ArcEntity : public Entity
    {
    public:
        ArcEntity(ObjectID id, const XMFLOAT3& center, float radius,  float startAngle, float endAngle)
            : Entity(id)
            , m_arc(center, radius, startAngle, endAngle)
        {
        } 
        const Arc& GetArc()         const { return m_arc; }
        AABB       GetBoundingBox() const override { return m_arc.GetBounds(); }
        DECLARE_RUNTIME_TYPE(ArcEntity, Entity)

    private:
        Arc m_arc;
    }; 
}