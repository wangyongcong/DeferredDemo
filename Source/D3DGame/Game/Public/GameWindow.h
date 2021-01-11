#pragma once
#include "D3DGame.h"

namespace wyc
{
	class D3DGAME_API CGameWindow
	{
	public:
		static std::shared_ptr<CGameWindow> CreateGameWindow(HINSTANCE hInstance, const wchar_t* title, uint32_t width, uint32_t height);

		CGameWindow(HWND hWindow);
		virtual ~CGameWindow();
		virtual void Destroy();
		virtual void SetVisible(bool bIsVisible);
		virtual bool IsWindowValid() const;

	protected:
		HWND mWindowHandle;
	};
}