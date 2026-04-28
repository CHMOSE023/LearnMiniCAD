#pragma once 
#include"pch.h"
#include <memory>  
#include "Render/D3D11/Device.h"
#include "Render/D3D11/SwapChain.h"
#include "Render/D3D11/Renderer.h"
#include "Render/Viewport/Viewport.h"
#include "Editor/Editor.h"
#include "Input/InputSystem.h"
#include "Document/Document.h"
#include "UI/UIManager.h"
#include <imgui.h>
#include "Document/DocumentManager.h"
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

		bool InitDocument(Renderer& renderer, int width, int height);
		// ── 渲染 ──────────────────────────────────────────────
		void RenderFrame();


		// ── ImGui光标样式 ─────────────────────────────────────
		HCURSOR ToWin32Cursor(ImGuiMouseCursor cursor);

	private:
		// 窗口
		HWND m_hwnd;

		// ── D3D11 层（硬件资源）──────────────────────────────
		std::unique_ptr<Device>       m_device;
		std::unique_ptr<SwapChain>    m_swapChain;
		std::unique_ptr<Renderer>     m_renderer;

		// 
	
		//std::unique_ptr<Document>     m_document;
		DocumentManager               m_docManager;
		InputSystem                   m_inputSystem;


		// ── ImGui ──────────────────────
		UIManager                     m_uiManager;
	};
}