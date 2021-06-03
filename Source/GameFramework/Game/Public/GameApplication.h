#pragma once

#include <string>
#include "GameFramework.h"
#include "IGameWindow.h"
#include "IRenderDevice.h"

namespace wyc
{
	class GameInstance;

	class GAME_FRAMEWORK_API GameApplication
	{
		static HINSTANCE sModuleHandle;
		static HINSTANCE sApplicationHandle;
		static GameApplication* sApplicationPtr;

	public:
		static bool CreateApplication(HINSTANCE hInstance, const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		static void DestroyApplication();
		static GameApplication* Get();

		static void SetModuleHandle(HINSTANCE hModuleInstance);

		GameApplication(const wchar_t* appName);
		virtual ~GameApplication();

		virtual void ShowWindow(bool visible);
		virtual void Run();
		virtual void Quit(int exitCode);
		virtual HINSTANCE GetApplicationHandle() const;
		virtual HINSTANCE GetApplicationModule() const;

	protected:
		virtual void StartLogger();
		virtual bool CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight);
		virtual bool CreateDevice();

		std::wstring mAppName;
		GameInstance* mpGameInstance;
		IGameWindow* mpWindow;
		IRenderDevice* mpDevice;
	};

} // namespace wyc