#pragma once
#include "IGameWindow.h"

namespace wyc
{
	struct SSwapChainDesc
	{
		uint32_t width;
		uint32_t height;
		uint32_t bufferCount;
	};

	class IRenderDevice
	{
	public:
		virtual bool Initialzie(IGameWindow* gameWindow) = 0;
		virtual void Render() = 0;
		virtual void Close() = 0;
		virtual bool CreateSwapChain(const SSwapChainDesc &Desc) = 0;
		virtual void Present() = 0;
	};
} // namespace wyc