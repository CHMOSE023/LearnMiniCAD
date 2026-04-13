#pragma once 
#include <windows.h>
#include <memory>   
#include "Input/InputSystem.h"
namespace MiniCAD
{
	class MainWindow
	{

	public :
		MainWindow();
		~MainWindow();
		bool Initialize(const wchar_t* title, int width, int height);
		void Run();


	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT EventProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		bool InitWindow(const wchar_t* title, int width, int height); 

		// ── 渲染 ──────────────────────────────────────────────
		void RenderFrame();

	private:
		// 窗口
		HWND m_hwnd;
  
		// ── 消息分发 ────────────────────────────────────────────
		InputSystem                   m_input;
	};
}