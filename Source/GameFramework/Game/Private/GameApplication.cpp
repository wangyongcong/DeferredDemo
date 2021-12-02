#include "GameFrameworkPCH.h"

#include <algorithm>
#include <filesystem>

#include "GameApplication.h"
#include "GameInstance.h"
#include "LogMacros.h"
#include "Utility.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WindowsGameWindow.h"
#include "DeviceD3D12.h"
#endif

#include "IMemory.h"

extern bool MemAllocInit();
extern void MemAllocExit();

namespace wyc
{
	HINSTANCE GameApplication::sModuleHandle = NULL;
	HINSTANCE GameApplication::sApplicationHandle = NULL;
	GameApplication* GameApplication::sApplicationPtr;

	bool GameApplication::CreateApplication(HINSTANCE hInstance, const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		MemAllocInit();
		sApplicationHandle = hInstance;
		sApplicationPtr = tf_new(GameApplication, appName);
		sApplicationPtr->StartLogger();
		if (!sApplicationPtr->CreateGameWindow(windowWidth, windowHeight))
		{
			LogError("Fail to create game window");
			return false;
		}
		if (!sApplicationPtr->CreateDevice())
		{
			LogError("Fail to create device");
			return false;
		}
		sApplicationPtr->ShowWindow(true);
		Log("Application started");
		return true;
	}

	void GameApplication::DestroyApplication()
	{
		if(sApplicationPtr)
		{
			tf_delete(sApplicationPtr);
			sApplicationPtr = nullptr;
			Log("Application exited");
		}
		close_logger();
		MemAllocExit();
	}

	GameApplication* GameApplication::Get()
	{
		return sApplicationPtr;
	}

	void GameApplication::SetModuleHandle(HINSTANCE hModuleInstance)
	{
		sModuleHandle = hModuleInstance;
	}

	void GameApplication::Run()
	{
		MSG msg = {0};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			mpDevice->Render();
		}
		mpDevice->Close();
	}

	void GameApplication::Quit(int exitCode)
	{
		PostQuitMessage(exitCode);
	}

	HINSTANCE GameApplication::GetApplicationHandle() const
	{
		return sApplicationHandle;
	}

	HINSTANCE GameApplication::GetApplicationModule() const
	{
		return sModuleHandle != NULL ? sModuleHandle : sApplicationHandle;
	}

	GameApplication::GameApplication(const wchar_t* appName)
		: mAppName(appName)
		, mpWindow(nullptr)
		, mpDevice(nullptr)
		, mpGameInstance(nullptr)
	{

	}

	GameApplication::~GameApplication()
	{
		GameInstance::DestroyGameInstance();

		if (mpDevice)
		{
			mpDevice->Release();
			tf_delete(mpDevice);
			mpDevice = nullptr;
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

	bool GameApplication::CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight)
	{
		if (!mpWindow)
		{
			mpWindow = tf_new(WindowsGameWindow);
			return mpWindow->CreateGameWindow(mAppName.c_str(), windowWidth, windowHeight);
		}
		return true;
	}

	bool GameApplication::CreateDevice()
	{
		if (mpDevice)
		{
			return true;
		}
		if (!mpWindow)
		{
			return false;
		}
		mpDevice = tf_new(RenderDeviceD3D12);
		if (!mpDevice->Initialize(mpWindow))
		{
			return false;
		}

		return true;
	}

} // namespace wyc

