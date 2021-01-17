#pragma once

#include <d3d12.h>


namespace wyc
{
	class CGameWindow;
	
	class CRenderDeviceD3D12
	{
	public:
		CRenderDeviceD3D12();
		virtual ~CRenderDeviceD3D12();

		bool Initialzie(CGameWindow* gameWindow);
		virtual void Render();

	protected:
		
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height);

		bool mInitialized;
		uint8_t mFrameBuffCount;
		uint8_t mCurrentBackBufferIndex;
		uint32_t mDescriptorSizeRTV;
		uint64_t* mFrameFenceValues;
		uint64_t mFenceValue;

		ComPtr<ID3D12Device2> mDevice;
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
