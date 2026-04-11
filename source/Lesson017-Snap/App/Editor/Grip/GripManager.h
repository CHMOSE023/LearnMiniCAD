#pragma once
#include "Grip.h"
#include "App/Scene/Scene.h"
#include "Render/Viewport/Camera.h"
#include <vector>
#include <unordered_set>

namespace MiniCAD
{
    class GripManager
    {
    public:
        // 选集变化后调用，重建夹点列表
        void Rebuild(const std::unordered_set<Object::ObjectID>& selection, Scene* scene);

        // 屏幕坐标命中测试，返回夹点索引；未命中返回 -1
        int HitTest(const DirectX::XMFLOAT2& screenPt, const Camera* cam, float thresh = 8.f) const;

        const std::vector<Grip>& GetGrips() const { return m_grips; }

        bool Empty() const { return m_grips.empty(); }

    private:
        std::vector<Grip> m_grips;
    };
}
