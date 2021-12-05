#pragma once
#include "IGameWindow.h"

namespace wyc
{
	struct SwapChainDesc
	{
		uint32_t width;
		uint32_t height;
		uint32_t bufferCount;
	};

	class IRenderer
	{
	public:
		virtual bool Initialize(IGameWindow* gameWindow) = 0;
		virtual void Release() = 0;
		virtual void BeginFrame() = 0;
		virtual bool CreateSwapChain(const SwapChainDesc &Desc) = 0;
		virtual void Present() = 0;
		virtual void Close() = 0;
	};
} // namespace wyc