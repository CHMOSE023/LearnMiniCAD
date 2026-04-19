#pragma once
#include "App/Tools/ITool.h" 
#include "App/Scene/Scene.h" 
#include "App/CommandStack/CommandStack.h" 
#include "App/Command/AddEntityCommand.h" 
#include "Render/Viewport/Viewport.h"
#include <cstdio>
#include <DirectXMath.h>
#include <optional>  // 可选值容器
namespace MiniCAD
{
    using namespace DirectX;

    class LineTool : public ITool
    {
    public:
        LineTool(Scene& scene, CommandStack& cmdStack, Viewport& viewport)
            : m_scene(scene)
            , m_cmdStack(cmdStack)
            , m_viewport(viewport)
        {
            printf("[LineTool] 左键起点 | 左键延续 | 右键结束段 | 空格继续 | ESC 退出\n");
        } 
        ~LineTool()
        {
            printf("退出绘制\n");
        }

        bool OnInput(const InputEvent& e) override
        {
            if (e.IsLeftClick())
            {
                auto pt = GetPoint(e);

                if (!m_hasStart)
                {
                    m_start = pt;
                    m_hasStart = true;
                }
                else
                {
                    Commit(m_start, pt);
                    m_start = pt; // 连续画
                }
                return true;
            }

            if (e.IsRightClick() || e.IsCancel())
            {
                if (OnFinished) OnFinished();
                return true;
            }

            if (e.Type == InputEventType::MouseMove && m_hasStart)
            {
                m_preview = GetPoint(e); // 预览终点
                return false; // 交给渲染
            }

            return false;
        }

    private: 

        DirectX::XMFLOAT3 GetPoint(const InputEvent& e)
        {
            if (e.HasSnap) return e.SnapWorld;

            return m_viewport.GetCamera().ScreenToWorld(e.MouseX, e.MouseY);
        }

        void Commit(const XMFLOAT3& a, const XMFLOAT3& b)
        {
            auto id = m_scene.NextObjectID();

            auto line = std::make_unique<LineEntity>(id, a, b);

            auto cmd = std::make_unique<AddEntityCommand>(std::move(line));
            m_cmdStack.Execute(std::move(cmd), m_scene);

            printf("提交线\n");
        }
         
    private:
        Scene&        m_scene;
        CommandStack& m_cmdStack;
        Viewport&     m_viewport;

        bool m_hasStart = false;
        XMFLOAT3 m_start{};
        XMFLOAT3 m_preview{}; // ⭐ 动态预览（MiniCAD关键）

    };
}