#pragma once

#include <d3d12.h>
#include "IRenderer.h"


namespace wyc
{
	class RendererD3D12;

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
		void BeginFrame() override;
		void EndFrame() override;
		void Present() override;
		void Resize() override;
		void Close() override;
		const GpuInfo& GetGpuInfo(int index) override;
		// IRenderer

	protected:
		void EnableDebugLayer();
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height, const RendererConfig& config);
		void ReportLiveObjects(const wchar_t* prompt=nullptr);
		void WaitForComplete(uint64_t frameIndex);

		ERenderDeviceState mDeviceState;
		uint8_t mFrameBufferCount;
		uint8_t mFrameBufferIndex;
		uint8_t mSampleCount;
		uint8_t mSampleQuality;
		uint64_t mFrameIndex;
		unsigned mDescriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		DXGI_FORMAT mColorFormat;
		DXGI_FORMAT mDepthFormat;

		HWND mWindowHandle;
		D3D12GpuInfo mGpuInfo;
		ID3D12Debug* mpDebug;
		IDXGIFactory6* mpDXGIFactory;
		IDXGIAdapter4* mpAdapter;
		ID3D12Device2* mpDevice;
		ID3D12InfoQueue* mpDeviceInfoQueue;
		ID3D12DescriptorHeap* mpRtvHeap;
		ID3D12DescriptorHeap* mpDsvHeap;
		IDXGISwapChain4* mpSwapChain;
		ID3D12Resource** mppSwapChainBuffers;
		ID3D12Resource* mpDepthBuffer;

		ID3D12CommandQueue* mpCommandQueue;
		ID3D12GraphicsCommandList* mpCommandList;
		ID3D12CommandAllocator** mppCommandAllocators;
		ID3D12Fence* mpFrameFence;
		HANDLE mFrameFenceEvent;
		uint64_t* mpFrameFenceValue;
	};

} // namespace wyc
