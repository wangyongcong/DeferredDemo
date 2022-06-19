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
	WindowsApplication::WindowsApplication(const wchar_t* appName)
		: GameApplication(appName)
	{

	}
}
