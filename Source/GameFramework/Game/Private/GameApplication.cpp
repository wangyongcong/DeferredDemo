#include "GameFrameworkPCH.h"

#include <algorithm>
#include <filesystem>

#include "GameApplication.h"
#include "IMemory.h"
#include "IGameInstance.h"
#include "LogMacros.h"
#include "Utility.h"

#ifdef PLATFORM_WINDOWS
#include "WindowsApplication.h"
#include "WindowsWindow.h"
using ApplicationClass = wyc::WindowsApplication;
using GameWindowClass = wyc::WindowsWindow;
#endif

#ifdef RENDERER_DIRECT3D12
#include "RendererD3D12.h"
using RenderClass = wyc::RendererD3D12;
#endif

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
		if(!gApplication->Initialize(appName, windowWidth, windowHeight))
		{
			tf_delete(gApplication);
			return false;
		}
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

	bool GameApplication::Initialize(const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		mpWindow = tf_new(GameWindowClass);
		if(!mpWindow->Create(appName, windowWidth, windowHeight))
		{
			return false;
		}
		mpRenderer = tf_new(RenderClass);
		const RendererConfig config {
			3,
			1,
			TinyImageFormat_R8G8B8A8_SRGB,
			TinyImageFormat_D24_UNORM_S8_UINT,
#ifdef _DEBUG
			true,
#else
			false,
#endif
		};
		if(!mpRenderer->Initialize(mpWindow, config))
		{
			LogError("Fail to initialize renderer");
			return false;
		}
		mpWindow->SetVisible(true);
		return true;
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

} // namespace wyc
