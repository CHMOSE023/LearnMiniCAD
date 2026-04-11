#pragma once 
#include <windows.h>
#include <memory>  
#include "Render/D3D11/Device.h"
#include "Render/D3D11/SwapChain.h"
#include "Render/D3D11/Renderer.h"
#include "Input/InputSystem.h" 
#include "App/Document/Document.h" 

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

		// ── 初始化分步 ────────────────────────────────────────
		bool InitWindow(const wchar_t* title, int width, int height);
		bool InitD3D11(int width, int height); 
		bool InitViewportAndDocument(int width, int height);

		// ── 渲染 ──────────────────────────────────────────────
		void RenderFrame();

	private:
		// 窗口
		HWND m_hwnd; 

		// ── D3D11 层（硬件资源）──────────────────────────────
		std::unique_ptr<Device>       m_device;
		std::unique_ptr<SwapChain>    m_swapChain;
		std::unique_ptr<Renderer>     m_renderer; 

		// ── 渲染层（抽象渲染接口）────────────────────────────
		std::unique_ptr<Viewport>     m_viewport;

		// ── 应用层（文档、场景、编辑器等）──────────────────────
		std::unique_ptr<Document>     m_document; 
		InputSystem                   m_input;
		 
	};
}