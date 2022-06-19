#pragma once
#include "IGameWindow.h"

namespace wyc
{
	struct GpuInfo
	{
		std::wstring venderName;
		uint32_t vendorId;
		uint32_t deviceId;
		size_t videoMemory;
	};

	struct SwapChainDesc
	{
		uint32_t width;
		uint32_t height;
		uint32_t bufferCount;
	};

	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		virtual bool Initialize(IGameWindow* gameWindow) = 0;
		virtual void Release() = 0;
		virtual void BeginFrame() = 0;
		virtual bool CreateSwapChain(const SwapChainDesc &desc) = 0;
		virtual void Present() = 0;
		virtual void Close() = 0;
		virtual const GpuInfo& GetGpuInfo(int index=0) = 0;
	};
} // namespace wyc