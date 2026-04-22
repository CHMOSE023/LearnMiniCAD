#pragma once 
#include "App/Input/InputEvent.h"
#include <functional>
namespace MiniCAD
{ 
    class ITool
    {
    public:
        virtual ~ITool() = default;

        virtual bool OnInput(const InputEvent& e) = 0;

        virtual void Cancel() {}

        std::function<void()> OnFinished;
    };
}