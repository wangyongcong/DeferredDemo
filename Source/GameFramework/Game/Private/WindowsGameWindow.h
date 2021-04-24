#pragma once
#include "GameFramework.h"
#include "IGameWindow.h"

namespace wyc
{
	class GameFramework_API CWindowsGameWindow : public IGameWindow
	{
	public:
		CWindowsGameWindow();
		virtual ~CWindowsGameWindow();

		// Implement IGameWinodw
		virtual bool CreateGameWindow(const wchar_t* title, uint32_t width, uint32_t height) override;
		virtual void Destroy()  override;
		virtual void SetVisible(bool bIsVisible) override;
		virtual bool IsWindowValid() const override;
		virtual void GetWindowSize(unsigned& width, unsigned& height) const override;
		// IGameWindow 

		inline HWND GetWindowHandle() const
		{
			return mWindowHandle;
		}

	protected:
		HWND mWindowHandle;
		unsigned mWindowWidth, mWindowHeight;
	};
}