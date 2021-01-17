#pragma once

#include <string>
#include "D3DGame.h"

namespace wyc
{
	class CGameWindow;
	class CGameInstance;
	class CRenderDeviceD3D12;

	class D3DGAME_API CGameApplication
	{
		static HINSTANCE sModuleHandle;
		static HINSTANCE sApplicationHandle;
		static std::shared_ptr<CGameApplication> sApplicationPtr;

	public:
		static bool CreateApplication(HINSTANCE hInstance, const wchar_t* appName, uint32_t windowWidth, uint32_t windowHeight);
		static void DestroyApplication();
		static std::shared_ptr<CGameApplication> Get();

		static void SetModuleHandle(HINSTANCE hModuleInstance);

		CGameApplication(const wchar_t* appName);
		virtual ~CGameApplication();

		virtual void ShowWindow(bool bVisible);
		virtual void Run();
		virtual void Quit(int exitCode);
		virtual HINSTANCE GetApplicationHandle() const;
		virtual HINSTANCE GetApplicationModule() const;

	protected:
		virtual void StartLogger();
		virtual bool CreateGameWindow(uint32_t windowWidth, uint32_t windowHeight);
		virtual bool CreateDevice();

		std::wstring mAppName;
		std::shared_ptr<CGameWindow> mWindow;
		std::shared_ptr<CGameInstance> mGameInstance;
		std::shared_ptr<CRenderDeviceD3D12> mDevice;
	};

} // namespace wyc