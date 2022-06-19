#pragma once
#include "GameFramework.h"
#include "IGameWindow.h"

namespace wyc
{
	class GAME_FRAMEWORK_API WindowsWindow : public IGameWindow
	{
	public:
		WindowsWindow();
		virtual ~WindowsWindow();

		// Implement IGameWinodw
		bool CreateGameWindow(const wchar_t* title, uint32_t width, uint32_t height) override;
		void Destroy()  override;
		void SetVisible(bool bIsVisible) override;
		bool IsWindowValid() const override;
		void GetWindowSize(unsigned& width, unsigned& height) const override;
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