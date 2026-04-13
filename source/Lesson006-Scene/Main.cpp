#include <iostream> 
#define NOMINMAX
#include <Windows.h>
#include "App/Scene/Scene.h"
#include "Core/Entity/LineEntity.hpp"
#include "Core/Entity/CircleEntity.hpp"
#include <memory>
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
    
    Scene scene; 

    scene.AddEntity(std::make_unique<LineEntity>  (1, XMFLOAT3(0, 0, 0), XMFLOAT3(3, 4, 0))); 
    scene.AddEntity(std::make_unique<LineEntity>  (2, XMFLOAT3(6, 5, 0), XMFLOAT3(3, 4, 0)));
    scene.AddEntity(std::make_unique<CircleEntity>(3, XMFLOAT3(6, 5, 0), 6));
     
    scene.ForEachObject([&](const Object& obj)
    {
        if (obj.IsKindOf<LineEntity>())
        { 
            const LineEntity* line = static_cast<const LineEntity*>(&obj);

            PrintFloat3("line start",line->GetLine().Start);
            PrintFloat3("line end",line->GetLine().End);
             
        }

        if (obj.IsKindOf<CircleEntity>())
        {
            const CircleEntity* circle = static_cast<const CircleEntity*>(&obj);

            PrintFloat3("circle center", circle->GetCircle().Center);
        }

    });

    return 0;
}