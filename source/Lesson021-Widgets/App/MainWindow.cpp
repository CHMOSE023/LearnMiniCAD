#include "MainWindow.h"
#include <ErrorReporter.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace MiniCAD
{
	MainWindow::MainWindow() 
		: m_hwnd(0)
		, m_device(nullptr)
		, m_swapChain(nullptr)
		, m_renderer(nullptr)
	{}

	MainWindow::~MainWindow()
	{}

	bool MainWindow::Initialize(const wchar_t* title, int width, int height)
	{
		if (!InitWindow(title, width, height))
			return false;

		RECT rc;
		GetClientRect(m_hwnd, &rc);

		int clientW = rc.right  - rc.left;
		int clientH = rc.bottom - rc.top;
		  
		if (!InitD3D11(clientW, clientH)) 
			return false; 

		if (!InitViewportAndDocument(clientW, clientH)) 
			return false;

		return m_uiManager.Init(m_hwnd,m_device->GetDevice(), m_device->GetContext());
		 
	}

	void MainWindow::Run()
	{
		MSG msg = {};

		bool needsRedraw = true;

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				needsRedraw = true;
			}
			else
			{
				if (needsRedraw)
				{
					RenderFrame();
					
					needsRedraw = false;
				}
				else
				{
					WaitMessage();
				}
			}
		}

	}

	LRESULT MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		MainWindow* pThis = nullptr;

		if (msg == WM_NCCREATE)
		{
			auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
			pThis = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		}
		else
		{
			pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		}

		if (pThis)
			return pThis->EventProc(hwnd, msg, wParam, lParam);

		return DefWindowProcW(hwnd, msg, wParam, lParam); 
	}

	HCURSOR  MainWindow::ToWin32Cursor(ImGuiMouseCursor cursor)
	{
		switch (cursor)
		{
		case ImGuiMouseCursor_Arrow:        return LoadCursor(NULL, IDC_ARROW);
		case ImGuiMouseCursor_TextInput:    return LoadCursor(NULL, IDC_IBEAM);
		case ImGuiMouseCursor_ResizeAll:    return LoadCursor(NULL, IDC_SIZEALL);
		case ImGuiMouseCursor_ResizeEW:     return LoadCursor(NULL, IDC_SIZEWE);
		case ImGuiMouseCursor_ResizeNS:     return LoadCursor(NULL, IDC_SIZENS);
		case ImGuiMouseCursor_ResizeNESW:   return LoadCursor(NULL, IDC_SIZENESW);
		case ImGuiMouseCursor_ResizeNWSE:   return LoadCursor(NULL, IDC_SIZENWSE);
		case ImGuiMouseCursor_Hand:         return LoadCursor(NULL, IDC_HAND);
		case ImGuiMouseCursor_NotAllowed:   return LoadCursor(NULL, IDC_NO);
		default:                            return LoadCursor(NULL, IDC_ARROW);
		}
	}

	LRESULT MainWindow::EventProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{   
		// 消息先过 ImGui
		if (ImGui::GetCurrentContext() != nullptr)
		{
			ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);			
		}

		switch (msg)
		{
		case WM_SIZE:
		{
			UINT w = LOWORD(lParam), h = HIWORD(lParam);
			if (m_swapChain) m_swapChain->Resize(w, h); 
			if (m_viewport)  m_viewport->Resize(w, h);
			if (m_document)  m_document->GetEditor().OnResize(w, h);
				
			return 0;
		}
		case WM_SETCURSOR:
		{    
			if (LOWORD(lParam) == HTCLIENT)
			{
				if (ImGui::GetCurrentContext())
				{
					// ImGuiLayer.cpp 禁止imgui 争夺光标
					// ImGuiIO& io = ImGui::GetIO();
					// io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
					bool imguiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered();

					if (imguiHovered)
					{
						ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
						SetCursor(ToWin32Cursor(imguiCursor));
						return TRUE;
					}
				} 

				SetCursor(nullptr); // CAD 区：隐藏系统光标
				return TRUE;
			}

			return DefWindowProc(hwnd, msg, wParam, lParam);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		// ───────────── 输入消息交给 InputSystem ─────────────
		case WM_MBUTTONDOWN: 
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
		{
			// 被ImGui捕获时，不转发给 CAD
			bool imguiWantsMouse = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
			if (!imguiWantsMouse)
				m_input.Dispatch(hwnd, msg, wParam, lParam);
			return 0;
		}
		case WM_KEYDOWN:
		case WM_KEYUP: 
		{
			// 键盘被 ImGui 捕获时，不转发给 CAD
			bool imguiWantsKeyboard = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
			if (!imguiWantsKeyboard)
				m_input.Dispatch(hwnd, msg, wParam, lParam);
			return 0;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		}
	}

	bool MainWindow::InitWindow(const wchar_t* title, int width, int height)
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);

		WNDCLASSEXW wc = {};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = nullptr;
		wc.lpszClassName = L"MiniCADMainWindows1";
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = wc.hIcon;

		// 1. 注册窗口
		RegisterClassEx(&wc);

		RECT rc = { 0, 0, width, height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

		// 2. 创建窗口
		m_hwnd = CreateWindowEx(
			0,
			L"MiniCADMainWindows1",
			title,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rc.right - rc.left,
			rc.bottom - rc.top,
			nullptr,
			nullptr,
			hInstance,
			this); // 参数this


		// 3.显示窗口
		ShowWindow(m_hwnd, SW_SHOW);

		// 4.更新窗口
		UpdateWindow(m_hwnd);

		// 注册错误处理器
		SetErrorHandler([this](const std::string& msg)
			{
				int len = MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), -1, nullptr, 0);
				std::wstring wmsg(len, 0);
				MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), -1, wmsg.data(), len);
				MessageBox(m_hwnd, wmsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
			});


		return m_hwnd != nullptr;


	}

	bool MainWindow::InitD3D11(int width, int height)
	{
		m_device = std::make_unique<Device>();       // 设备
		m_device->Initialize();

		m_swapChain = std::make_unique<SwapChain>(); // 交换链

		SwapChain::Options opt;
		opt.enableVSync = false;  // 禁止垂直同步，允许撕裂（仅限窗口模式）
		opt.allowTearing = false; // 允许撕裂（仅限窗口模式）
		 
		m_swapChain->Initialize(m_device.get(), m_hwnd, width, height, opt);// 初始化交换链
		m_renderer = std::make_unique<Renderer>(m_device->GetDevice(), m_device->GetContext());

		return true;
	}

	bool MainWindow::InitViewportAndDocument(int width, int height)
	{  
		m_viewport = std::make_unique<Viewport>(m_renderer.get(), width, height); // 传入m_renderer

		m_document = std::make_unique<Document>(width, height);

		m_viewport->SetCamera(m_document->GetScene().GetCamera()); // 设置相机

		m_input.PushHandler(m_document.get());         // 先注册 Document，保证它优先处理输入事件

		m_input.PushHandler(&m_document->GetEditor()); // 先注册 Document，保证它优先处理输入事件

		// 添加一些测试数据
		auto& scene = m_document->GetScene();
		// 直线1
		auto id = scene.NextObjectID();
		auto entity = std::make_unique<LineEntity>(id,XMFLOAT3(1.5, 1.5, 0), XMFLOAT3(0, 0, 0));
		entity->GetAttr().Color = XMFLOAT4(1, 1, 0, 1); 
		scene.AddEntity(std::move(entity));

		// 直线2
		id = scene.NextObjectID();
		entity = std::make_unique<LineEntity>(id, XMFLOAT3(0.5, 0.5, 0), XMFLOAT3(1, 0, 0));
		entity->GetAttr().Color = XMFLOAT4(1, 0, 0, 1);
		scene.AddEntity(std::move(entity));

		return true;
	}

	void MainWindow::RenderFrame()
	{
		auto  target    = m_swapChain->GetRenderTarget(); 		 
		auto& scene     = m_document->GetScene();
		auto  viewState = m_document->GetEditor().BuildViewState();


		m_viewport->RefreshRenderData(m_document->GetScene(), viewState);

		m_viewport->Render(target);
		
		{  // ui 窗口
			m_uiManager.BeginFrame();
			 
			m_uiManager.Render(*m_document);

			m_uiManager.EndFrame();
		}

		m_swapChain->Present();

	}

}
