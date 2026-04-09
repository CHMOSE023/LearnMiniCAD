#include "MainWindow.h"

using namespace MiniCAD;
int main()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);


	MainWindow mainWindow;

	mainWindow.Initialize(L"Lesson012-Preview",600,400);
	mainWindow.Run();

}