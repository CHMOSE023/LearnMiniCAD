#pragma once
#include "App/CommandStack/ICommand.h"
#include "Core/Object/Object.hpp"
#include "Core/Entity/LineEntity.hpp"
#include "App/Editor/Grip/MoveGrip.h"
#include <memory> 
namespace MiniCAD
{ 
    class DragLineCommand : public ICommand
    {
    public:
        DragLineCommand(Object::ObjectID id, const  LineSegment& before, const  LineSegment& after)
            : m_id(id)
            , m_before(before)
            , m_after(after) 
        {
        } 

        void Execute(Scene& scene) override
        {
            Apply(scene, m_after);
        }

        void Undo(Scene& scene) override
        {
            Apply(scene, m_before);
        }

        std::string GetName() const override { return "拖动直线"; }
    private:
        Object::ObjectID m_id;
        LineSegment m_before;
        LineSegment m_after; 
        void Apply(Scene& scene, const LineSegment& seg) const
        {
            auto* line = static_cast<LineEntity*>(scene.GetEntity(m_id));
            if (line)
            { 
                line->SetLine({ seg.Start,seg.End });  
            }
               
        }
    };

}
