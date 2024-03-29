#pragma once
#include "GameFramework.h"
#include "IGameWindow.h"
#include "Utility.h"

namespace wyc
{
	class GAME_FRAMEWORK_API WindowsWindow : public IGameWindow
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(WindowsWindow)
	public:
		WindowsWindow();
		~WindowsWindow() override;

		// Implement IGameWinodw
		bool Create(const wchar_t* title, uint32_t width, uint32_t height) override;
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
