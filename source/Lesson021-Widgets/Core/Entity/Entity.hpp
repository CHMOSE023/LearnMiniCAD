#pragma once  
#include <DirectXMath.h>
#include "Core/ISerializer.h"  
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

        virtual AABB GetBoundingBox() const = 0;             // 获取包围盒
        virtual void Serialize(ISerializer& s) const = 0;    // 将对象写入序列化器       
        virtual void Deserialize(ISerializer& s) = 0;        // 从序列化器读取对象状态
    protected:
        explicit Entity(ObjectID id) : Object(id) {}
    private:
        EntityAttr m_attr;
    };

}
