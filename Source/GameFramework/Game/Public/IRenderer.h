#pragma once
#include "IGameWindow.h"
#include "tinyimageformat/tinyimageformat_base.h"

namespace wyc
{
	struct RendererConfig
	{
		uint8_t frameBufferCount;
		uint8_t sampleCount;
		TinyImageFormat colorFormat;
		TinyImageFormat depthStencilFormat;
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
		int msaa4QualityLevel;
		int msaa8QualityLevel;
	};

	enum EFenceStatus
	{
		Complete,
		Incomplete,
	};

	enum ECommandType
	{
		Draw,
		Compute,
		Copy,
		// count of command list type
		MaxCount
	};

	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		virtual bool Initialize(IGameWindow* gameWindow, const RendererConfig& config) = 0;
		virtual void Release() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;
		virtual void Resize() = 0;
		virtual void Close() = 0;
		virtual const GpuInfo& GetGpuInfo(int index=0) = 0;
	};
} // namespace wyc