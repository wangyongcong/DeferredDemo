#pragma once

#include <d3d12.h>
#include "IRenderDevice.h"


namespace wyc
{
	enum
	{
		MAX_GPU_VENDOR_STRING_LENGTH = 64
	};

	struct FFence
	{
		ID3D12Fence* mpDxFence;
		HANDLE mhWaitEvent;
		uint64_t mFenceValue;
		uint64_t mPadA;
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
		virtual void SwapBuffer() override;
		// IRenderDevice

	protected:
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height);
		bool AddCommandQueue();
		bool NewFence(FFence*& outFence);
		void ReleaseFence(FFence*& pFence);
		void EnableDebugLayer();
		void Signal();
		void WaitForFence();

		bool mInitialized;
		uint8_t mFrameBuffCount;
		uint8_t mCurrentBackBufferIndex;
		uint32_t mDescriptorSizeRTV;
		uint64_t* mFrameFenceValues;
		uint64_t mFenceValue;

		ID3D12Debug* mpDebug;
		IDXGIFactory6* mpDXGIFactory;
		IDXGIAdapter4* mpAdapter;
		ID3D12Device2* mpDevice;
		ID3D12CommandQueue* mpCommandQueue;
		FFence* mQueueFence;
		ComPtr<IDXGISwapChain4> mSwapChain;
		ComPtr<ID3D12DescriptorHeap> mSwapChainHeap;
		ComPtr<ID3D12Resource>* mBackBuffers;
		ComPtr<ID3D12CommandAllocator>* mCommandAllocators;
		ComPtr<ID3D12GraphicsCommandList> mCommandList;
		ComPtr<ID3D12Fence> mFence;
		HANDLE mFenceEvent;

		struct
		{
			D3D_FEATURE_LEVEL featureLevel = (D3D_FEATURE_LEVEL)0;
			D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
			D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureData1;
			size_t DedicatedVideoMemory = 0;
			uint32_t VendorId;
			uint32_t DeviceId;
			uint32_t Revision;
			std::wstring Name;
		} GpuInfo;
	};

} // namespace wyc
