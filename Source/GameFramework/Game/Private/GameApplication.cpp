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

namespace wyc
{
	HINSTANCE CGameApplication::sModuleHandle = NULL;
	HINSTANCE CGameApplication::sApplicationHandle = NULL;
	CGameApplication* CGameApplication::sApplicationPtr;

	bool CGameApplication::CreateApplication(HINSTANCE hInstance, const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight)
	{
		sApplicationHandle = hInstance;
		sApplicationPtr = new CGameApplication(appName);
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

	void CGameApplication::DestroyApplication()
	{
		if(sApplicationPtr)
		{
			delete sApplicationPtr;
			sApplicationPtr = nullptr;
			Log("Application exited");
		}
		close_logger();
	}

	CGameApplication* CGameApplication::Get()
	{
		return sApplicationPtr;
	}

	void CGameApplication::SetModuleHandle(HINSTANCE hModuleInstance)
	{
		sModuleHandle = hModuleInstance;
	}

	void CGameApplication::Run()
	{
		MSG msg = {0};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			mDevice->Render();
		}
		mDevice->Close();
	}

	void CGameApplication::Quit(int exitCode)
	{
		PostQuitMessage(exitCode);
	}

	HINSTANCE CGameApplication::GetApplicationHandle() const
	{
		return sApplicationHandle;
	}

	HINSTANCE CGameApplication::GetApplicationModule() const
	{
		return sModuleHandle != NULL ? sModuleHandle : sApplicationHandle;
	}

	CGameApplication::CGameApplication(const wchar_t* appName)
		: mAppName(appName)
		, mWindow(nullptr)
		, mDevice(nullptr)
		, mGameInstance(nullptr)
	{

	}

	CGameApplication::~CGameApplication()
	{
		CGameInstance::DestroyGameInstance();

		if (mDevice)
		{
			delete mDevice;
		}
		if(mWindow)
		{
			delete mWindow;
		}
	}

	void CGameApplication::ShowWindow(bool bVisible)
	{
		if(mWindow)
		{
			mWindow->SetVisible(bVisible);
		}
	}

	void CGameApplication::StartLogger()
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

	bool CGameApplication::CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight)
	{
		if (!mWindow)
		{
			mWindow = new CWindowsGameWindow;
			return mWindow->CreateGameWindow(mAppName.c_str(), windowWidth, windowHeight);
		}
		return true;
	}

	bool CGameApplication::CreateDevice()
	{
		if (mDevice)
		{
			return true;
		}
		if (!mWindow)
		{
			return false;
		}
		mDevice = new CRenderDeviceD3D12;
		if (!mDevice->Initialzie(mWindow))
		{
			return false;
		}

		return true;
	}

} // namespace wyc

