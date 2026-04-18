#pragma once
#include <vector>
#include <functional>
#include "Core/Object/Object.hpp"  
namespace MiniCAD
{
    class LayerManager;
    // Scene 数据访问接口（解耦 Scene）
    class ISceneReader
    {
    public:
        using ObjectID = Object::ObjectID;

        virtual ~ISceneReader() = default;
        virtual const Object* GetEntity(ObjectID id) const = 0;

        virtual bool Has(ObjectID id) const = 0;
        virtual void ForEachObject(std::function<void(const Object&)>) const = 0;   // 枚举（核心接口）	
        virtual void ForEachPreview(std::function<void(const Object&)>) const = 0;  // 遍历 预览对象     

        virtual int   EntityCount() const = 0;                     // 信息    
        virtual const LayerManager& GetLayerManager() const = 0;   // Layer       
        virtual bool  IsDirty() const = 0;                         // 状态

    };

}