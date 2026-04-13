#include "Core/Entity/LineEntity.hpp"
#include <iostream>
#include <Windows.h>

using namespace MiniCAD;
using namespace DirectX;

// 辅助：打印 XMFLOAT3
static void PrintFloat3(const char* label, const XMFLOAT3& v)
{
    std::cout << label << ": (" << v.x << ", " << v.y << ", " << v.z << ")\n";
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ── 1. 构造 LineEntity ────────────────────────────────
    LineEntity line1(1, XMFLOAT3(0, 0, 0), XMFLOAT3(3, 4, 0));
    std::cout << "=== 创建 line1 ===" << std::endl;
    std::cout << "ID: " << line1.GetID() << std::endl;

    // ── 2. 读取几何信息 ───────────────────────────────────
    const Line& l = line1.GetLine();
    PrintFloat3("Start", l.Start);
    PrintFloat3("End", l.End);
    std::cout << "Length: " << l.Length() << std::endl;   // 应输出 5
    PrintFloat3("Midpoint", l.Midpoint());

    // ── 3. 最近点 / 距离计算 ──────────────────────────────
    XMFLOAT3 testPt(1, 3, 0);
    PrintFloat3("ClosestPoint to (1,3,0)", l.ClosestPoint(testPt));                // 点到线段的垂足
    std::cout << "DistanceTo (1,3,0): " << l.DistanceToPoint(testPt) << std::endl; // 点到线段距离精确为

    // ── 4. 包围盒 ─────────────────────────────────────────
    AABB box = line1.GetBoundingBox();
    PrintFloat3("AABB.Min", box.Min);
    PrintFloat3("AABB.Max", box.Max);

    // ── 5. 修改属性 ───────────────────────────────────────
    std::cout << "\n=== 修改属性 ===" << std::endl;
    EntityAttr& attr = line1.GetAttr();
    attr.Color = { 1.0f, 0.0f, 0.0f, 1.0f };   // 改为红色
    attr.LineWidth = 2.5f;
    attr.Visible = false;
    attr.LineType = LineType::DASHED;
    attr.LayerId = 3;

    // 读回验证
    const EntityAttr& a = line1.GetAttr();
    std::cout << "Color RGBA: (" << a.Color.x << ", " << a.Color.y << ", " << a.Color.z << ", " << a.Color.w << ")\n";
    std::cout << "LineWidth: " << a.LineWidth << std::endl;
    std::cout << "Visible: " << a.Visible << std::endl;
    std::cout << "LayerId: " << a.LayerId << std::endl;
    std::cout << "LineType: " << static_cast<int>(a.LineType) << " (1=DASHED)\n";

    // ── 6. SetAttr 整体替换 ───────────────────────────────
    EntityAttr newAttr;
    newAttr.Color = { 0.0f, 1.0f, 0.0f, 1.0f };  // 绿色
    newAttr.LineWidth = 1.0f;
    newAttr.Visible = true;
    line1.SetAttr(newAttr);
    std::cout << "\n=== SetAttr 后 Color.g: " << line1.GetAttr().Color.y << " (应为1.0) ===\n";

    // ── 7. IsKindOf 类型检查 ──────────────────────────────
    std::cout << "\n=== 类型检查 ===" << std::endl;
    std::cout << "IsKindOf<LineEntity>: " << line1.IsKindOf<LineEntity>() << std::endl;
    std::cout << "IsKindOf<Object>:     " << line1.IsKindOf<Object>() << std::endl;

    // ── 8. 第二条线 ───────────────────────────────────────
    LineEntity line2(2, XMFLOAT3(0, 1, 0), XMFLOAT3(0, 1, 0));
    std::cout << "\nline2 IsValid: " << line2.GetLine().IsValid() << " (起止相同，应为0)" << std::endl;

    return 0;
}