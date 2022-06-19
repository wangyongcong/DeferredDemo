#include "GameFrameworkPCH.h"

#include <algorithm>
#include <filesystem>

#include "GameApplication.h"
#include "IGameInstance.h"
#include "LogMacros.h"
#include "Utility.h"

#ifdef PLATFORM_WINDOWS
#include "WindowsApplication.h"
using ApplicationClass = wyc::WindowsApplication;
#endif

#include "IMemory.h"

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	GAME_FRAMEWORK_API GameApplication* gApplication = nullptr;

	bool GameApplication::CreateApplication(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		MemAllocInit();
		gApplication = tf_new(ApplicationClass, appName);
		gApplication->StartLogger();
		IGameWindow* window = gApplication->CreateGameWindow(windowWidth, windowHeight);
		if (!window)
		{
			LogError("Fail to create game window");
			tf_delete(gApplication);
			return false;
		}
		gApplication->mpWindow = window;
		IRenderer* renderer = gApplication->CreateRenderer();
		if (!renderer)
		{
			LogError("Fail to create device");
			tf_delete(gApplication);
			return false;
		}
		gApplication->mpRenderer = renderer;
		window->SetVisible(true);
		Log("Application started");
		return true;
	}

	void GameApplication::DestroyApplication()
	{
		if(gApplication)
		{
			tf_delete(gApplication);
			gApplication = nullptr;
			Log("Application exited");
		}
		close_logger();
		MemAllocExit();
	}

	static int64_t gHighResTimerFrequency = 0;

	void InitTime()
	{
		LARGE_INTEGER frequency;
		if (QueryPerformanceFrequency(&frequency))
		{
			gHighResTimerFrequency = frequency.QuadPart;
		}
		else
		{
			gHighResTimerFrequency = 1000LL;
		}
	}

	int64_t GetTime()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart * (int64_t)1e6 / gHighResTimerFrequency;
	}

	float GetTimeSince(int64_t &start)
	{
		int64_t end = GetTime();
		float delta = (float)(end - start) / (float)1e6;
		start = end;
		return delta;
	}

	void GameApplication::QuitGame(int exitCode)
	{
		PostQuitMessage(exitCode);
	}

	GameApplication::GameApplication(const wchar_t* appName)
		: mAppName(appName)
		, mpGameInstance(nullptr)
		, mpWindow(nullptr)
		, mpRenderer(nullptr)
	{

	}

	GameApplication::~GameApplication()
	{
		if (mpRenderer)
		{
			mpRenderer->Release();
			tf_delete(mpRenderer);
			mpRenderer = nullptr;
		}
		if(mpWindow)
		{
			mpWindow->Destroy();
			tf_delete(mpWindow);
			mpWindow = nullptr;
		}
	}

	void GameApplication::ShowWindow(bool visible)
	{
		if(mpWindow)
		{
			mpWindow->SetVisible(visible);
		}
	}

	void GameApplication::StartGame(IGameInstance* pGame)
	{
		mpGameInstance = pGame;
		InitTime();
		mpGameInstance->Init();
		int64_t lastTime = GetTime(), currentTime;
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			float deltaTime = GetTimeSince(lastTime);
			mpGameInstance->Tick(deltaTime);
			mpRenderer->BeginFrame();
			mpGameInstance->Draw(mpRenderer);
			mpRenderer->Present();
		}
		mpRenderer->Close();
		mpGameInstance->Exit();
	}

	void GameApplication::StartLogger()
	{
		auto path = std::filesystem::current_path();
		path /= "Saved";
		path /= "Logs";
		std::wstring logName = mAppName + L".log";
		std::replace(logName.begin(), logName.end(), L' ', L'-');
		path /= logName.c_str();
		std::string ansi = path.generic_string();
		start_file_logger(ansi.c_str());
	}

	IGameWindow* GameApplication::CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight)
	{
		return nullptr;
	}

	IRenderer* GameApplication::CreateRenderer()
	{
		return nullptr;
	}

} // namespace wyc
