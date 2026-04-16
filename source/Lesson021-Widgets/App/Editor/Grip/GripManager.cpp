#include "GripManager.h"
#include "Core/Entity/LineEntity.hpp"
#include <cmath>

using namespace DirectX;

namespace MiniCAD
{
    void GripManager::Rebuild(const std::unordered_set<Object::ObjectID>& sel, Scene* scene)
    {
        m_grips.clear();
		auto cam = scene->GetCamera();
        scene->ForEachObject([&](const Object& obj)
        {
            if (!sel.count(obj.GetID())) return;

            auto* line = dynamic_cast<const LineEntity*>(&obj);
            if (!line) return;

            auto& L = line->GetLine();

            m_grips.push_back({ obj.GetID(), Grip::Type::Start, L.Start });
            m_grips.push_back({ obj.GetID(), Grip::Type::Mid,   { (L.Start.x + L.End.x) * 0.5f, (L.Start.y + L.End.y) * 0.5f ,0} });
            m_grips.push_back({ obj.GetID(), Grip::Type::End,   L.End });
        });
    }

    int GripManager::HitTest(const XMFLOAT2& sp, const Camera* cam, float thresh) const
    {
        int   bestIdx = -1;
        float bestDist = FLT_MAX;

        for (int i = 0; i < (int)m_grips.size(); ++i)
        {
            XMFLOAT2 sc = cam->WorldToScreen(m_grips[i].WorldPos);
            float d = std::hypot(sp.x - sc.x, sp.y - sc.y);

            if (d < thresh && d < bestDist)
            {
                bestDist = d;
                bestIdx = i;
            }
        }

        return bestIdx;
    }
}