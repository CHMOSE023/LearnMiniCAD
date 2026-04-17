#pragma once  
#include <DirectXMath.h>
#include "Core/Object/Object.hpp"  
#include "Core/GeomKernel/AABB.hpp"  
#include "EntityAttr.hpp"
using namespace DirectX; 

namespace MiniCAD
{
    class Entity : public Object
    {
    public:
        EntityAttr&       GetAttr()                    { return m_attr; }
        const EntityAttr& GetAttr() const              { return m_attr; }
        void              SetAttr(const EntityAttr& a) { m_attr = a; }
        LayerID           GetLayerID() const           { return m_attr.LayerId; }
        void              SetLayerId(LayerID id)       { m_attr.LayerId = id; }

        virtual AABB      GetBoundingBox() const = 0;
    protected:
        explicit Entity(ObjectID id) : Object(id) {}
    private:
        EntityAttr m_attr;
    };

}
