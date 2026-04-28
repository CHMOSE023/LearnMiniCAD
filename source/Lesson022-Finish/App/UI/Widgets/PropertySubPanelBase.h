#pragma once
namespace MiniCAD
{
    class Document;
    class Object;

    class PropertySubPanelBase
    {
    public:
        virtual ~PropertySubPanelBase() = default;
        virtual void OnRender(Object* entity, Document& document) = 0;
    };
}
