#pragma once  
#include <DirectXMath.h>
#include "Core/Object/Object.hpp"
#include "Core/Object/RuntimeType.hpp"
#include "Core/GeomKernel/Polyline.hpp" 
#include "Entity.hpp"
using namespace DirectX;

namespace MiniCAD
{    
    class PolylineEntity : public Entity
    {
    public:
        PolylineEntity(ObjectID id, std::vector<XMFLOAT3> pts, bool closed = false)
            : Entity(id)
            , m_poly(std::move(pts), closed) 
        {
        }

        const Polyline& GetPolyline() const { return m_poly; }
        AABB GetBoundingBox() const override { return m_poly.GetBounds(); }
        DECLARE_RUNTIME_TYPE(PolylineEntity, Entity)
    private:
        Polyline m_poly;
    };

}