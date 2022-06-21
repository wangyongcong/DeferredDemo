#include "GameFrameworkPCH.h"
#include "resource.h"
#include "WindowsPlatform.h"
#include "WindowsWindow.h"

#include "LogMacros.h"

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

	bool WindowsWindow::Create(const wchar_t* title, uint32_t width, uint32_t height)
	{
		static const wchar_t* sWindowClassName = L"MainGameWindow";
		static bool sIsClassRegistered = false;

		if(!sIsClassRegistered)
		{
			WNDCLASSEX windowClass;
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = &WindowProcess;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = gAppInstance;

			HINSTANCE moduleInstance = GetModuleInstance();
			windowClass.hIcon = LoadIcon(moduleInstance, MAKEINTRESOURCE(IDI_APP_ICON));
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowClass.hbrBackground = HBRUSH(COLOR_WINDOW);
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = sWindowClassName;
			windowClass.hIconSm = LoadIcon(moduleInstance, MAKEINTRESOURCE(IDI_APP_ICON));

			HRESULT hr = RegisterClassEx(&windowClass);
			if (!SUCCEEDED(hr))
			{
				// fail to register window
				LogError("Fail to register windows class %s", sWindowClassName);
				return nullptr;
			}
			sIsClassRegistered = true;
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

		HWND hMainWnd = CreateWindowW(sWindowClassName, title, style,
			windowX, windowY, windowWidth, windowHeight, NULL, NULL, gAppInstance, NULL);
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

	void WindowsWindow::SetVisible(bool bIsVisible)
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

	bool WindowsWindow::IsWindowValid() const
	{
		return IS_WINDOW_VALID(mWindowHandle);
	}

	void WindowsWindow::GetWindowSize(unsigned& width, unsigned& height) const
	{
		width = mWindowWidth;
		height = mWindowHeight;
	}

	WindowsWindow::WindowsWindow()
		: mWindowHandle(NULL)
		, mWindowWidth(0)
		, mWindowHeight(0)
	{

	}

	WindowsWindow::~WindowsWindow()
	{
		if(mWindowHandle != NULL)
		{
			DestroyWindow(mWindowHandle);
			mWindowHandle = NULL;
		}
	}

}
