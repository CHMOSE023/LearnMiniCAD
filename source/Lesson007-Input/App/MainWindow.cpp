#include "MainWindow.h" 
namespace MiniCAD
{
	MainWindow::MainWindow() 
		: m_hwnd(0) 
	{}

	MainWindow::~MainWindow()
	{}

	bool MainWindow::Initialize(const wchar_t* title, int width, int height)
	{
		InitWindow(title,width,height);
		  
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

	LRESULT MainWindow::EventProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{ 
		switch (msg)
		{
		case WM_SIZE:
		{
			UINT w = LOWORD(lParam), h = HIWORD(lParam); 
			printf("WM_SIZE:%d,%d\n", w, h);
			return 0;
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
		case WM_KEYDOWN:
		case WM_KEYUP:
			m_input.Dispatch(hwnd, msg, wParam, lParam);
			return 0; 
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
		 
		return m_hwnd != nullptr; 
	}

	  
	void MainWindow::RenderFrame()
	{  
	}

}
