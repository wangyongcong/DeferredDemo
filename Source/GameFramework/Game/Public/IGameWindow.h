#pragma once

namespace wyc
{
	class IGameWindow
	{
	public:
		virtual ~IGameWindow() = default;
		virtual bool Create(const wchar_t* title, uint32_t width, uint32_t height) = 0;
		virtual void Destroy() = 0;
		virtual void SetVisible(bool bIsVisible) = 0;
		virtual bool IsWindowValid() const = 0;
		virtual void GetWindowSize(unsigned &width, unsigned &height) const = 0;
	};
} // namespace wyc
