#pragma once  
#include <DirectXMath.h>
#include "Core/Object/Object.hpp"
#include "Core/Object/RuntimeType.hpp"
#include "Core/GeomKernel/Line.hpp" 
#include "Entity.hpp"
using namespace DirectX;

namespace MiniCAD
{ 
	class LineEntity: public Entity
	{ 
	public: 
		LineEntity(ObjectID id, const XMFLOAT3& start, const XMFLOAT3& end) : Entity(id), m_line(start, end), m_attr{} {};

		void        SetLine(const Line& line) { m_line = line; }
		const Line& GetLine() const { return m_line; }
 
		LayerID GetLayerID() const { return m_attr.LayerId; }
		void    SetLayerId(LayerID id) { m_attr.LayerId = id; }

		virtual	AABB GetBoundingBox() const { return m_line.GetBounds(); };
		  
		DECLARE_RUNTIME_TYPE(LineEntity, Object) 

	private:
		Line       m_line; 
		EntityAttr m_attr;
	};  


}