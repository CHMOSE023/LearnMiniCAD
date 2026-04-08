#include "MainWindow.h"

using namespace MiniCAD;
int main()
{

	MainWindow mainWindow;

	mainWindow.Initialize(L"Lesson010-Document",600,400);
	mainWindow.Run();

}