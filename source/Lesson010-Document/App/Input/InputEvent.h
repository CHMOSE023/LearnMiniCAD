#pragma once
#include <cstdint> 

namespace MiniCAD
{
    enum class InputEventType : uint8_t
    {
        None,
        MouseButtonDown,
        MouseButtonUp,
        MouseMove,
        MouseWheel,
        KeyDown,
        KeyUp,
    };

    enum class MouseButton : uint8_t { None, Left, Middle, Right };

    enum class ModifierKey : uint8_t
    {
        None  = 0,
        Shift = 1 << 0,
        Ctrl  = 1 << 1,
        Alt   = 1 << 2,
    };

    struct InputEvent
    {
        InputEventType Type      = InputEventType::None;
        MouseButton    Button    = MouseButton::None;
        uint8_t        Modifiers = 0;                    // ModifierKey 位掩码
       
        int   MouseX     = 0;     // 客户区像素坐标
        int   MouseY     = 0;
		int   LastMouseX = 0;     //上一个事件的坐标 
		int   LastMouseY = 0;    

        float    WheelDelta = 0.f;
        uint32_t KeyCode = 0;

        bool HasModifier(ModifierKey k) const
        {
            return (Modifiers & static_cast<uint8_t>(k)) != 0;
        }  
    };  
}