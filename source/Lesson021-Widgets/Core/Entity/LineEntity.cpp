#include "LineEntity.hpp"
#include "Core/Entity/ObjectFactory.hpp"
#include "EntityAttr.hpp"
using namespace DirectX;

namespace MiniCAD
{
    // ── 工厂注册 ────────────────────────────────────────────
    static bool s_registered = []()
    {
            ObjectFactory::Get().Register("LineEntity", [](ISerializer& s)
                {
                    auto obj = std::make_unique<LineEntity>(0, XMFLOAT3{}, XMFLOAT3{});
                    obj->Deserialize(s);
                    return obj;
                });
            return true;
    }();

    // ── Serialize ────────────────────────────────────────────
    void LineEntity::Serialize(ISerializer& s) const
    {
        s.WriteString("LineEntity");

        s.WriteFloat(m_line.Start.x);
        s.WriteFloat(m_line.Start.y);
        s.WriteFloat(m_line.Start.z);
        s.WriteFloat(m_line.End.x);
        s.WriteFloat(m_line.End.y);
        s.WriteFloat(m_line.End.z);

        auto attr =  GetAttr();
        s.WriteFloat(attr.Color.x);
        s.WriteFloat(attr.Color.y);
        s.WriteFloat(attr.Color.z);
        s.WriteFloat(attr.Color.w);

        s.WriteUInt64(attr.LayerId);
        s.WriteUInt64(static_cast<uint64_t>(attr.LineType));
        s.WriteFloat (attr.LineWidth);
        s.WriteBool  (attr.Visible);
    }

    // ── Deserialize ──────────────────────────────────────────
    void LineEntity::Deserialize(ISerializer& s)
    {
        m_line.Start.x = s.ReadFloat();
        m_line.Start.y = s.ReadFloat();
        m_line.Start.z = s.ReadFloat();
        m_line.End.x = s.ReadFloat();
        m_line.End.y = s.ReadFloat();
        m_line.End.z = s.ReadFloat();

        EntityAttr attr = {};

        attr.Color.x = s.ReadFloat();
        attr.Color.y = s.ReadFloat();
        attr.Color.z = s.ReadFloat();
        attr.Color.w = s.ReadFloat();

        attr.LayerId = static_cast<LayerID>(s.ReadUInt64());
        attr.LineType = static_cast<LineType>(s.ReadUInt64());
        attr.LineWidth = s.ReadFloat();
        attr.Visible = s.ReadBool();

        SetAttr(attr);
    }
}
