#pragma once  
#include "pch.h"
#include "Render/D3D11/Device.h"
#include "Render/D3D11/SwapChain.h"
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Viewport.h"
#include "Input/InputSystem.h"
#include "Editor/Editor.h"
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
		bool InitD3D11(int width, int height);

		// ── 渲染 ──────────────────────────────────────────────
		void RenderFrame();

	private:
		// 窗口
		HWND m_hwnd;

		// ── D3D11 层（硬件资源）──────────────────────────────
		std::unique_ptr<Device>       m_device;
		std::unique_ptr<SwapChain>    m_swapChain;
		std::unique_ptr<Renderer>     m_renderer;
		//  
		std::unique_ptr<Viewport>     m_viewport; 

		 
		// ── 调试Editor Scene CommandStack m_inputSystem ─────
		std::unique_ptr<Editor>       m_editor;
		std::unique_ptr<Scene>        m_scene;
		std::unique_ptr<CommandStack> m_cmdStack;
		InputSystem                   m_inputSystem;
	};
}