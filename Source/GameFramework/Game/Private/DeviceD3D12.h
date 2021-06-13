#pragma once

#include <d3d12.h>
#include "IRenderDevice.h"


namespace wyc
{
	enum
	{
		MAX_GPU_VENDOR_STRING_LENGTH = 64
	};


	struct DeviceFence
	{
		ID3D12Fence* mpDxFence = nullptr;
		HANDLE mhWaitEvent = NULL;
		uint64_t mFenceValue = 0;
		uint64_t mPadA = 0;
	};

	struct GPUInfo
	{
		D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
		D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureData1;
		size_t DedicatedVideoMemory;
		uint32_t VendorId;
		uint32_t DeviceId;
		uint32_t Revision;
		std::wstring Name;
	};

	class RenderDeviceD3D12 : public IRenderDevice
	{
	public:
		RenderDeviceD3D12();
		virtual ~RenderDeviceD3D12();

		// Implement IRenderDevice
		virtual bool Initialzie(IGameWindow* gameWindow) override;
		virtual void Render() override;
		virtual void Close() override;
		virtual bool CreateSwapChain(const SSwapChainDesc& Desc) override;
		virtual void Present() override;
		// IRenderDevice

	protected:
		void EnableDebugLayer();
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height);
		bool CreateCommandQueue();
		bool CreateCommandList();
		bool CreateFence(DeviceFence& outFence);
		void ReleaseFence(DeviceFence& pFence);
		void WaitForFence(DeviceFence& pFence);

		bool mInitialized;
		uint8_t mMaxFrameLatency;
		uint64_t mFrameCount;
		uint32_t mFrameIndex;
		uint32_t mBackBufferIndex;
		uint32_t mDescriptorSizeRTV;

		ID3D12Debug* mpDebug;
		IDXGIFactory6* mpDXGIFactory;
		IDXGIAdapter4* mpAdapter;
		ID3D12Device2* mpDevice;
		ID3D12CommandQueue* mpCommandQueue;
		DeviceFence mQueueFence;

		ID3D12GraphicsCommandList* mpCommandList;
		ID3D12CommandAllocator** mppCommandAllocators;
		DeviceFence* mpCommandFences;

		ComPtr<IDXGISwapChain4> mSwapChain;
		ComPtr<ID3D12DescriptorHeap> mSwapChainHeap;
		ComPtr<ID3D12Resource>* mBackBuffers;

		GPUInfo mGpuInfo;
	};

} // namespace wyc
