#pragma once
#include "D3DGame.h"

namespace wyc
{
	class D3DGAME_API CGameWindow
	{
	public:
		CGameWindow();
		virtual ~CGameWindow();
		virtual bool CreateGameWindow(const wchar_t* title, uint32_t width, uint32_t height);
		virtual void Destroy();
		virtual void SetVisible(bool bIsVisible);
		virtual bool IsWindowValid() const;
		
		inline HWND GetWindowHandle() const
		{
			return mWindowHandle;
		}
		inline void GetWindowSize(unsigned& width, unsigned& height) const
		{
			width = mWindowWidth;
			height = mWindowHeight;
		}

	protected:
		HWND mWindowHandle;
		unsigned mWindowWidth, mWindowHeight;
	};
}