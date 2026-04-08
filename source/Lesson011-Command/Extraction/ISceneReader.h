#pragma once
#include <vector>
#include <functional>
#include "Core/Object/Object.hpp" 
#include "Render/Viewport/Camera.h"
namespace MiniCAD
{   
	class LayerManager;
    // Scene 数据访问接口（解耦 Scene）
    class ISceneReader
    {
    public:
        using ObjectID = Object::ObjectID;

        virtual ~ISceneReader() = default;

        // 单体查询
        virtual const Object* GetEntity(ObjectID id) const = 0;
        virtual bool Has(ObjectID id) const = 0;

        // 枚举（核心接口）
        virtual void ForEachObject(std::function<void(const Object&)>) const = 0;

        // 信息
        virtual int EntityCount() const = 0;

        // Layer
        virtual const LayerManager& GetLayerManager() const = 0;

        // 状态
        virtual bool IsDirty() const = 0;

		// 获取相机（如果有的话，某些场景可能没有相机）
        virtual Camera* GetCamera()  const = 0;
    };

}