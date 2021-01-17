#include "D3DGamePCH.h"
#include "resource.h"
#include "GameWindow.h"
#include "GameApplication.h"

namespace wyc
{
#define IS_WINDOW_VALID(Handle) ((Handle) != NULL)

	LRESULT CALLBACK WindowProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		PAINTSTRUCT ps;
		HDC hdc;
		switch (message)
		{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: custom GUI paint 
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	bool CGameWindow::CreateGameWindow(const wchar_t* title, uint32_t width, uint32_t height)
	{
		const wchar_t* WindowClassName = L"D3DGameWindow";

		auto pAppInstance = CGameApplication::Get();
		auto appHandle = pAppInstance->GetApplicationHandle();
		auto appModule = pAppInstance->GetApplicationModule();

		WNDCLASSEX windowClass;
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = &WindowProcess;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = appHandle;

		windowClass.hIcon = LoadIcon(appModule, MAKEINTRESOURCE(IDI_APP_ICON));
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = HBRUSH(COLOR_WINDOW);
		windowClass.lpszMenuName = NULL;
		windowClass.lpszClassName = WindowClassName;
		windowClass.hIconSm = LoadIcon(appModule, MAKEINTRESOURCE(IDI_APP_ICON));

		HRESULT hr = RegisterClassEx(&windowClass);
		if (!SUCCEEDED(hr))
		{
			// fail to register window
			return nullptr;
		}

		DWORD style = WS_OVERLAPPEDWINDOW;
		RECT clientRect = { 0, 0, int(width), int(height) };
		AdjustWindowRect(&clientRect, style, FALSE);
		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;

		int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
		int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

		HWND hMainWnd = CreateWindowW(WindowClassName, title, style,
			windowX, windowY, windowWidth, windowHeight, NULL, NULL, appHandle, NULL);
		if (!hMainWnd)
		{
			// fail to create windows
			return false;
		}
		
		mWindowHandle = hMainWnd;
		mWindowWidth = windowWidth;
		mWindowHeight = windowHeight;
		return true;
	}

	void CGameWindow::Destroy()
	{
		if(mWindowHandle != NULL)
		{
			DestroyWindow(mWindowHandle);
			mWindowHandle = NULL;
		}
	}

	void CGameWindow::SetVisible(bool bIsVisible)
	{
		if(!IS_WINDOW_VALID(mWindowHandle))
		{
			return;
		}
		if(bIsVisible)
		{
			ShowWindow(mWindowHandle, SW_NORMAL);
		}
		else
		{
			ShowWindow(mWindowHandle, SW_HIDE);
		}
	}

	bool CGameWindow::IsWindowValid() const
	{
		return IS_WINDOW_VALID(mWindowHandle);
	}

	CGameWindow::CGameWindow()
		: mWindowHandle(NULL)
		, mWindowWidth(0)
		, mWindowHeight(0)
	{

	}

	CGameWindow::~CGameWindow()
	{
		Destroy();		
	}

}