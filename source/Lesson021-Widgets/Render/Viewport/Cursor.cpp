#include "Cursor.h"
using namespace DirectX;

namespace MiniCAD
{
    void Cursor::Build(const ViewState& state, float width, float  height)
    {
        m_verts.clear();
        
        float half = 5.0f;
        float x    = state.MouseX;
        float y    = state.MouseY;
         
        m_verts.push_back({ XMFLOAT3(0,          y, 0), XMFLOAT4(1,1,1,1) });
        m_verts.push_back({ XMFLOAT3(width,      y, 0), XMFLOAT4(1,1,1,1) });
        m_verts.push_back({ XMFLOAT3(x,     height, 0), XMFLOAT4(1,1,1,1) });
        m_verts.push_back({ XMFLOAT3(x,          0, 0), XMFLOAT4(1,1,1,1) }); 

		// 中间的方框（如果正在框选）
        if (!state.BoxSelected && state.CrossBox)
        {
            m_verts.push_back({ XMFLOAT3(x - half, y - half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x + half, y - half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x + half, y - half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x + half, y + half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x + half, y + half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x - half, y + half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x - half, y + half, 0), XMFLOAT4(1,1,1,1) });
            m_verts.push_back({ XMFLOAT3(x - half, y - half, 0), XMFLOAT4(1,1,1,1) });
             
        }

        if (state.BoxSelected)
        {
            // 二 、框选矩形
            float x0 = state.BoxPressX;
            float y0 = state.BoxPressY;
            float x1 = state.MouseX;
            float y1 = state.MouseY;

            // 4边矩形（线框） 
            XMFLOAT4 color = x1 > x0 ? XMFLOAT4{ 0.0f, 0.8f, 0.5f, 1.0f } : XMFLOAT4{ 0.5f, 0.8f, 0.0f, 1.0f };

            m_verts.push_back({ {x0, y0, 0}, color });
            m_verts.push_back({ {x1, y0, 0}, color });

            m_verts.push_back({ {x1, y0, 0}, color });
            m_verts.push_back({ {x1, y1, 0}, color });

            m_verts.push_back({ {x1, y1, 0}, color });
            m_verts.push_back({ {x0, y1, 0}, color });

            m_verts.push_back({ {x0, y1, 0}, color });
            m_verts.push_back({ {x0, y0, 0}, color });

        }
		
    }
}