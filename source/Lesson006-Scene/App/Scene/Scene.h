#pragma once
#include "Core/Object/Object.hpp"
#include "Core/Entity/LineEntity.hpp"  
#include "Extraction/ISceneReader.h"
#include "App/Scene/LayerManager.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>
namespace MiniCAD
{    
	class Scene :public ISceneReader
	{ 
	public: 

		using ObjectID      = Object::ObjectID;
		using DirtyCallback = std::function<void()>;
		 
		Scene() = default;   

		void AddEntity(std::unique_ptr<Object> entity);       // 添加实体 	
		std::unique_ptr<Object> RemoveEntity(ObjectID id); 	  // 移除并返回所有权（供 Undo 使用）

		void AddPreviewEntity(std::unique_ptr<Object> entity);// 添加预览实体 	
		void ClearPreviews(); 
		 
		Object* GetEntity(ObjectID id);                     // 通过ID查询
		virtual const Object* GetEntity(ObjectID id) const override;

		virtual bool Has(ObjectID id) const override;

		std::vector<ObjectID> GetAllIDs() const;            // 返回所有实体 ID

		LayerManager& GetLayerManager() { return m_layerManager; }
		virtual const LayerManager& GetLayerManager() const override { return m_layerManager; } 
		
		virtual void  ForEachObject (std::function<void(const Object&)> fn) const;  // 遍历所有实体（核心接口，供 ISceneReader 实现）  
		virtual void  ForEachPreview(std::function<void(const Object&)> fn) const; // 遍历所有预览对象（核心接口，供 ISceneReader 实现）

		// ── DirtyFlag ── 
		virtual bool IsDirty() const override { return m_dirty; }
		void MarkDirty()     { m_dirty = true; if (m_onDirty) m_onDirty(); }
		void ClearDirty()    { m_dirty = false; }

		void SetDirtyCallback(DirtyCallback cb) { m_onDirty = std::move(cb); }

		virtual int EntityCount() const override { return (int)m_entities.size(); }
		   
		// ── 序列化 ───────────────────────────────────────────
		void Serialize  (ISerializer& s) const;
		void Deserialize(ISerializer& s);
		  
		// ── ID 分配 ────────────────────────────────────────── 
		ObjectID NextObjectID() { return m_nextObjectID.fetch_add(1, std::memory_order_relaxed); }
	private:     
		void SyncNextObjectID(); 

	private:
		std::unordered_map<ObjectID, std::unique_ptr<Object>> m_entities;  

		std::vector<std::unique_ptr<Object>> m_previews; // 预览对象 ID 列表（保持顺序）

		std::atomic<ObjectID>    m_nextObjectID{ 1 };    // 0 保留为 InvalidID
		   
		LayerManager  m_layerManager;
		bool          m_dirty = false;
		DirtyCallback m_onDirty;

	};
}