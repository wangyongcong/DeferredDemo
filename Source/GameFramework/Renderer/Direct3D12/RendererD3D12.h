#pragma once

#include <d3d12.h>
#include "IRenderer.h"


namespace wyc
{

	struct DeviceFence
	{
		ID3D12Fence* mpDxFence = nullptr;
		HANDLE mhWaitEvent = NULL;
		uint64_t mFenceValue = 0;
		uint64_t mPadA = 0;
	};

	struct D3D12GpuInfo : GpuInfo
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL(0);
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
		D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureData1;
	};

	enum class ERenderDeviceState : uint8_t
	{
		DEVICE_EMPTY = 0,
		DEVICE_INITIALIZED,
		DEVICE_CLOSED,
		DEVICE_RELEASED,
	};

	class RendererD3D12 : public IRenderer
	{
	public:
		RendererD3D12();
		~RendererD3D12() override;

		// Implement IRenderer
		bool Initialize(IGameWindow* gameWindow, const RendererConfig& config) override;
		void Release() override;
		bool CreateSwapChain(const SwapChainDesc& desc) override;
		void BeginFrame() override;
		void Present() override;
		void Close() override;
		const GpuInfo& GetGpuInfo(int index) override;
		// IRenderer

	protected:
		void EnableDebugLayer();
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height, const RendererConfig& config);
		bool CreateCommandQueue();
		bool CreateCommandList();
		bool CreateFence(DeviceFence& outFence);
		void ReleaseFence(DeviceFence& pFence);
		void WaitForFence(DeviceFence& pFence);
		void ReportLiveObjects(const wchar_t* prompt=nullptr);
		
	protected:
		ERenderDeviceState mDeviceState;
		uint8_t mMaxFrameLatency;
		uint64_t mFrameCount;
		uint32_t mFrameIndex;
		uint32_t mBackBufferIndex;
		uint32_t mDescriptorSizeRTV;

		ID3D12Debug* mpDebug;
		IDXGIFactory6* mpDXGIFactory;
		IDXGIAdapter4* mpAdapter;
		ID3D12Device2* mpDevice;
		ID3D12InfoQueue* mpDeviceInfoQueue;
		ID3D12CommandQueue* mpCommandQueue;
		DeviceFence mQueueFence;

		ID3D12GraphicsCommandList* mpCommandList;
		ID3D12CommandAllocator** mppCommandAllocators;
		DeviceFence* mpCommandFences;

		IDXGISwapChain4* mpSwapChain;
		ID3D12DescriptorHeap* mpSwapChainHeap;
		ID3D12Resource** mppBackBuffers;

		D3D12GpuInfo mGpuInfo;
	};

} // namespace wyc
