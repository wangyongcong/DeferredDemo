#include "D3DGamePCH.h"
#include "GameApplication.h"
#include "GameWindow.h"
#include "GameInstance.h"

namespace wyc
{
	std::shared_ptr<CGameApplication> CGameApplication::sApplicationPtr;

	bool CGameApplication::CreateApplication(HINSTANCE hInstance)
	{
		sApplicationPtr = std::make_shared<CGameApplication>(hInstance);
		return true;
	}

	void CGameApplication::DestroyApplication()
	{
	}

	std::shared_ptr<wyc::CGameApplication> CGameApplication::Get()
	{
		return sApplicationPtr;
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
		}
	}

	void CGameApplication::Quit(int exitCode)
	{
		PostQuitMessage(exitCode);
	}

	CGameApplication::CGameApplication(HINSTANCE hInstance)
		: mApplicationHandle(hInstance)
	{

	}

	CGameApplication::~CGameApplication()
	{
		CGameInstance::DestroyGameInstance();

		if(mWindow)
		{
			mWindow->Destroy();
			mWindow = nullptr;
		}
	}

	bool CGameApplication::ShowWindow(const wchar_t* title, uint32_t width, uint32_t height)
	{
		if(mApplicationHandle == NULL)
		{
			return false;
		}
		if(mWindow)
		{
			mWindow->Destroy();
		}
		mWindow = CGameWindow::CreateGameWindow(mApplicationHandle, title, width, height);
		if(!mWindow)
		{
			return false;
		}
		mWindow->SetVisible(true);
		return true;
	}

}