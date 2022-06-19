#pragma once
#include "GameApplication.h"

namespace wyc
{
	class WindowsApplication: public GameApplication
	{
	public:
		explicit WindowsApplication(const wchar_t* appName);

	protected:
		IGameWindow* CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight) override;
		IRenderer* CreateRenderer() override;
	};
}
