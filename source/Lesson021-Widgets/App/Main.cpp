#include "MainWindow.h"

using namespace MiniCAD;
int main()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);


	MainWindow mainWindow;

	mainWindow.Initialize(L"Lesson021-Widgets",600,400);
	mainWindow.Run();

}