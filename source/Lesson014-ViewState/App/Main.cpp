#include "pch.h"
#include "MainWindow.h"
using namespace MiniCAD;
int main()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);


	MainWindow mainWindow;

	mainWindow.Initialize(L"Lesson014-ViewState",600,400);
	mainWindow.Run();

}