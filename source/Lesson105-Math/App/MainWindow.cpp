#include "MainWindow.h"
#include <ErrorReporter.h>
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
		InitWindow(title,width,height);

		InitD3D11(width, height);

		return true;
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

	int LastMouseX = 0;
	int LastMouseY = 0;
	bool m_isPanning = false;
	LRESULT MainWindow::EventProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		
		POINT curPt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		switch (msg)
		{
		case WM_SIZE:
		{
			UINT w = LOWORD(lParam), h = HIWORD(lParam);
			if (m_swapChain) m_swapChain->Resize(w, h); 

			if (m_viewport)m_viewport->Resize(w, h);
			 
			return 0;
		}
		case WM_MOUSEMOVE:  
			if (m_isPanning && m_viewport) {
				int dx = curPt.x - LastMouseX;
				int dy = curPt.y - LastMouseY;
				m_viewport->Pan(dx, dy);
			}
			LastMouseX = curPt.x;
			LastMouseY = curPt.y;
			return 0;
		case WM_MBUTTONDOWN: // 鼠标按下
			m_isPanning = true;
			LastMouseX = curPt.x;
			LastMouseY = curPt.y;
			SetCapture(hwnd);   // 捕获鼠标，防止移出窗口丢失事件
			return 0;
		case WM_MBUTTONUP: // 鼠标抬起
			m_isPanning = false;
			ReleaseCapture();
			return 0;
		case WM_MOUSEWHEEL: 
		{ 

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ScreenToClient(hwnd, &pt); // ！！！屏幕坐标转为客户区

			int	mouseX = pt.x;
			int	mouseY = pt.y;
			float wheelDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;

			if (m_viewport)m_viewport->Zoom(wheelDelta, mouseX, mouseY);

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

		m_viewport = std::make_unique<Viewport>(m_renderer.get(), width, height);

		return true;
	}

	void MainWindow::RenderFrame()
	{
		auto target = m_swapChain->GetRenderTarget(); 
 
		 
		m_viewport->Render(target);
	 

		m_swapChain->Present();

	}

}
