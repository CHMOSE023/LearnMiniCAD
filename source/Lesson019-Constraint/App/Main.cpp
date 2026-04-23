#include "pch.h"
#include "MainWindow.h"
using namespace MiniCAD;
int main()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);


	MainWindow mainWindow;

	mainWindow.Initialize(L"Lesson019-Constraint 正交约束 ",600,400);
	mainWindow.Run();

}