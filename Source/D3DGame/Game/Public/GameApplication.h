#pragma once
#include "D3DGame.h"

namespace wyc
{
	class CGameWindow;
	class CGameInstance;

	class D3DGAME_API CGameApplication
	{
	public:
		static bool CreateApplication(HINSTANCE hInstance);
		
		static void DestroyApplication();

		static std::shared_ptr<CGameApplication> Get();

		CGameApplication(HINSTANCE hInstance);
		virtual ~CGameApplication();

		virtual bool ShowWindow(const wchar_t* title, uint32_t width, uint32_t height);
		virtual void Run();
		virtual void Quit(int exitCode);

	protected:
		static std::shared_ptr<CGameApplication> sApplicationPtr;

		HINSTANCE mApplicationHandle;
		std::shared_ptr<CGameWindow> mWindow;
		std::shared_ptr<CGameInstance> mGameInstance;
	};

} // namespace wyc