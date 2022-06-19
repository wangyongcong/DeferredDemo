#pragma once
#include <string>
#include <cstdint>
#include "Platform.h"
#include "GameFramework.h"
#include "IGameWindow.h"
#include "IRenderer.h"

namespace wyc
{
	class IGameInstance;

	class GAME_FRAMEWORK_API GameApplication
	{
	public:
		static bool CreateApplication(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		static void DestroyApplication();

		explicit GameApplication(const wchar_t* appName);
		virtual ~GameApplication();
		GameApplication(const GameApplication&) = delete;
		GameApplication& operator = (const GameApplication&) = delete;

		void ShowWindow(bool visible);
		void StartGame(IGameInstance* pGame);
		void QuitGame(int exitCode);

		inline IGameWindow* GetWindow() const
		{
			return mpWindow;
		}
		inline IRenderer* GetRenderer() const
		{
			return mpRenderer;
		}

	protected:
		virtual bool Initialize(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		virtual void StartLogger();

		std::wstring mAppName;
		IGameInstance* mpGameInstance;
		IGameWindow* mpWindow;
		IRenderer* mpRenderer;
	};

	extern GAME_FRAMEWORK_API GameApplication* gApplication;
} // namespace wyc

#ifdef PLATFORM_WINDOWS
#define APPLICATION_MAIN(GameInstanceClass) \
namespace wyc \
{\
	extern GAME_FRAMEWORK_API HINSTANCE gAppInstance;\
}\
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nShowCmd) \
{\
	wyc::gAppInstance = hInstance;\
	if(!wyc::GameApplication::CreateApplication(L#GameInstanceClass, 1290, 720)) {\
		return 1;\
	}\
	GameInstanceClass gameInstance;\
	gApplication->StartGame(&gameInstance);\
	gApplication->DestroyApplication();\
	return 0;\
}
#endif