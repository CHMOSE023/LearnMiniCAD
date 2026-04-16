#pragma once 
#include "App/Input/InputEvent.h"
#include <DirectXMath.h>
namespace MiniCAD
{
    class ITool
    {
    public:
        virtual ~ITool() = default;

        virtual void OnMouseDown(const InputEvent& e) = 0;
        virtual void OnMouseUp  (const InputEvent& e) = 0;
        virtual void OnMouseMove(const InputEvent& e) = 0;

        virtual void OnKeyDown  (const InputEvent& e) = 0;

        virtual bool OnMouseWheel(const InputEvent& e)
        {
            return false; // 默认不吃滚轮
        }

        virtual void Cancel() {}

        virtual bool HasAnchor() const { return false; }            // 是否有“锚点”
        virtual DirectX::XMFLOAT3 GetAnchor() const { return {}; }  // 获取锚点（仅在 HasAnchor() == true 时有效）
    };
}