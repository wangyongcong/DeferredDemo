#pragma once

#include <d3d12.h>
#include "IRenderDevice.h"


namespace wyc
{
	class CRenderDeviceD3D12 : public IRenderDevice
	{
	public:
		CRenderDeviceD3D12();
		virtual ~CRenderDeviceD3D12();

		// Implement IRenderDevice
		virtual bool Initialzie(IGameWindow* gameWindow) override;
		virtual void Render() override;
		virtual void Close() override;
		virtual bool CreateSwapChain(const SSwapChainDesc& Desc) override;
		virtual void SwapBuffer() override;
		// IRenderDevice

	protected:
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height);
		void Signal();
		void WaitForFence();

		bool mInitialized;
		uint8_t mFrameBuffCount;
		uint8_t mCurrentBackBufferIndex;
		uint32_t mDescriptorSizeRTV;
		uint64_t* mFrameFenceValues;
		uint64_t mFenceValue;

		ID3D12Device2* mDevice;
		ComPtr<ID3D12CommandQueue> mCommandQueue;
		ComPtr<IDXGISwapChain4> mSwapChain;
		ComPtr<ID3D12DescriptorHeap> mSwapChainHeap;
		ComPtr<ID3D12Resource>* mBackBuffers;
		ComPtr<ID3D12CommandAllocator>* mCommandAllocators;
		ComPtr<ID3D12GraphicsCommandList> mCommandList;
		ComPtr<ID3D12Fence> mFence;
		HANDLE hFenceEvent;
	};

} // namespace wyc
