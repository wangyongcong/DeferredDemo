#include "WindowsApplication.h"
#include <filesystem>
#include "IMemory.h"
#include "IGameInstance.h"
#include "LogMacros.h"
#include "WindowsWindow.h"
#include "RendererD3D12.h"

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	GAME_FRAMEWORK_API HINSTANCE gModuleInstance = NULL;
	GAME_FRAMEWORK_API HINSTANCE gAppInstance = NULL;

	WindowsApplication::WindowsApplication(const wchar_t* appName)
		: GameApplication(appName)
	{

	}

	IGameWindow* WindowsApplication::CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight)
	{
		WindowsWindow* window = tf_new(WindowsWindow);
		if(!window->CreateGameWindow(mAppName.c_str(), windowWidth, windowHeight))
		{
			tf_delete(window);
			return nullptr;
		}
		return window;
	}

	IRenderer* WindowsApplication::CreateRenderer()
	{
		RendererD3D12* renderer = tf_new(RendererD3D12);
		if (!renderer->Initialize(mpWindow))
		{
			tf_delete(renderer);
			return nullptr;
		}
		return renderer;
	}
}
