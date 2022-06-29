#pragma once
#include "IGameWindow.h"

namespace wyc
{
	struct RendererConfig
	{
		bool enableDebug;

	};

	#define GPU_VENDOR_NAME_SIZE 64

	struct GpuInfo
	{
		wchar_t vendorName[GPU_VENDOR_NAME_SIZE];
		unsigned vendorId;
		unsigned deviceId;
		unsigned revision;
		size_t videoMemory;
		size_t sharedMemory;
		int msaa4;
		int msaa8;
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
		virtual bool Initialize(IGameWindow* gameWindow, const RendererConfig& config) = 0;
		virtual void Release() = 0;
		virtual void BeginFrame() = 0;
		virtual bool CreateSwapChain(const SwapChainDesc &desc) = 0;
		virtual void Present() = 0;
		virtual void Close() = 0;
		virtual const GpuInfo& GetGpuInfo(int index=0) = 0;
	};
} // namespace wyc