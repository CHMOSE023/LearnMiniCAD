#pragma once
#include "App/Editor/Tools/ITool.h" 
#include "App/Scene/Scene.h"
#include "App/CommandStack/CommandStack.h"
#include "App/Input/InputEvent.h"
#include "Core/Entity/LineEntity.hpp"
#include "App/Command/AddEntityCommand.h"  
#include <optional>

namespace MiniCAD
{
    class LineTool : public ITool
    {
    public:
        LineTool(Scene* scene, CommandStack* cmd )
            : m_scene(scene)
            , m_cmd(cmd) 
        {
            printf("[LineTool] 左键起点 | 左键延续 | 右键结束段 | 空格继续 | ESC 退出\n");
        }

        ~LineTool()
        { 
            printf("[LineTool] 已退出\n");
        }

        void OnMouseDown(const InputEvent& e) override
        {
            if (e.Button == MouseButton::Left)
            {
                auto pt = ScreenToWorld(e);

                if (m_state == State::WaitFirstPoint)
                {
                    m_start = pt;
                    m_state = State::Drawing;
                    printf("[LineTool] 起点 (%.2f, %.2f)\n", pt.x, pt.y); 
                }
                else if (m_state == State::Drawing)
                {
                    CommitLine(m_start, pt);
                    m_start = pt; // 终点延续为下一段起点
                }
            }
            else if (e.Button == MouseButton::Right)
            {
                // 右键：结束当前连续段，等待新起点
                if (m_state == State::Drawing)
                { 
                    m_scene->ClearPreviews();
                    m_state = State::WaitFirstPoint;
                    printf("[LineTool] 右键结束，等待新起点\n");
                }
            }
        }

        void OnMouseUp(const InputEvent& e) override
        {
            // 连续绘制模式，MouseUp 不处理
        }

        void OnMouseMove(const InputEvent& e) override
        {
            if (m_state == State::Drawing)
            {
                UpdatePreview(ScreenToWorld(e));
            }
        }

        void OnKeyDown(const InputEvent& e) override
        {
            switch (e.KeyCode)
            {
            case VK_ESCAPE:         
                m_scene->ClearPreviews();
                m_lastCommittedPoint.reset();
                m_state = State::WaitFirstPoint;
                printf("[LineTool] ESC \n");
                break;

            case VK_SPACE:
				printf("[LineTool] Space \n");
                // 从上一个提交点继续绘制
                if (m_lastCommittedPoint.has_value())
                {
                    m_start = m_lastCommittedPoint.value();
                    m_state = State::Drawing;
                    printf("[LineTool] 空格继续，从 (%.2f, %.2f) 开始\n", m_start.x, m_start.y);
                }
                else
                {
                    printf("[LineTool] 没有上一个点\n");
                }
                break;
            }
        }

        void Cancel() override
        { 
            m_scene->ClearPreviews();
            m_lastCommittedPoint.reset();
            m_state = State::WaitFirstPoint;
        }

    private:
        enum class State { WaitFirstPoint, Drawing };

        State                    m_state = State::WaitFirstPoint;
        XMFLOAT3                 m_start = {};
        std::optional<XMFLOAT3>  m_lastCommittedPoint; 

        Scene*        m_scene; 
        CommandStack* m_cmd;

        // ── 工具函数 ──────────────────────────────────

        XMFLOAT3 ScreenToWorld(const InputEvent& e)
        {   
            // ！！如果捕获有效 使用捕获
            if (e.HasSnap)
                return e.SnapWorld;

            auto p =  m_scene->GetCamera()->ScreenToWorld(e.MouseX,e.MouseY);            
            return XMFLOAT3(p.x, p.y, 0.f); // CAD 永远在 XY 平面，强制 Z=0
        }

        void CommitLine(const XMFLOAT3& start, const XMFLOAT3& end)
        { 
			m_scene->ClearPreviews(); // 清除预览对象 

            auto layerId = m_scene->GetLayerManager().GetActiveLayerID();
            const auto* layer = m_scene->GetLayerManager().GetLayer(layerId);
            if (layer && layer->IsLocked())
            {
                ReportError("Cannot create entities on a locked layer.");
                m_state = State::WaitFirstPoint;
                return;
            }

            auto id = m_scene->NextObjectID(); 
            auto line = std::make_unique<LineEntity>(id, start, end);

            line->GetAttr().Visible = true;
            line->GetAttr().Color = XMFLOAT4(1.f, 1.f, 0.f, 1.f); 
			line->SetLayerId(layerId); 

            auto cmd = std::make_unique<AddEntityCommand>(std::move(line));
            m_cmd->Execute(std::move(cmd), *m_scene);

            m_lastCommittedPoint = end;

            printf("[LineTool] 提交 (%.2f,%.2f) → (%.2f,%.2f)\n", start.x, start.y, end.x, end.y);
        }

        void UpdatePreview(const XMFLOAT3& end)
        {
			m_scene->ClearPreviews(); // 清除之前的预览对象

            auto line = std::make_unique<LineEntity>(0, m_start, end);

            line->GetAttr().Color = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f);

            m_scene->AddPreviewEntity(std::move(line));
        }

        // 是否有“锚点”
        bool HasAnchor() const override
        {
            return m_state == State::Drawing;
        }

        // 返回锚点
        DirectX::XMFLOAT3 GetAnchor() const override
        {
            return DirectX::XMFLOAT3(m_start.x, m_start.y, 0.f);
        }
    };
}
 
