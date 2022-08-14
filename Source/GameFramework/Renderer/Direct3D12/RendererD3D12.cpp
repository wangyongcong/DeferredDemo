#include "GameFrameworkPCH.h"
#include "RendererD3D12.h"
#include <chrono>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgidebug.h>
// D3D12 extension library.
#include <d3dx12.h>

#include "AssertionMacros.h"
#include "LogMacros.h"
#include "WindowsWindow.h"
#include "D3DHelper.h"
#include "IMemory.h"


namespace wyc
{
	RendererD3D12::RendererD3D12()
		: mDeviceState(ERenderDeviceState::DEVICE_EMPTY)
		, mFrameBufferCount(3)
		, mFrameBufferIndex(0)
		, mSampleCount(1)
		, mSampleQuality()
		, mFrameIndex()
		, mDescriptorSize{}
		, mColorFormat()
		, mDepthFormat()
		, mWindowHandle(nullptr)
		, mGpuInfo()
		, mpDebug(nullptr)
		, mpDXGIFactory(nullptr)
		, mpAdapter(nullptr)
		, mpDevice(nullptr)
		, mpDeviceInfoQueue(nullptr)
		, mpRtvHeap(nullptr)
		, mpDsvHeap(nullptr)
		, mpSwapChain(nullptr)
		, mppSwapChainBuffers(nullptr)
		, mpDepthBuffer(nullptr)
		, mpCommandQueue(nullptr)
		, mpCommandList(nullptr)
		, mppCommandAllocators(nullptr)
		, mpFrameFence(nullptr)
		, mFrameFenceEvent(nullptr)
		, mpFrameFenceValue(nullptr)
	{
	}

	RendererD3D12::~RendererD3D12()
	{
		Release();
	}

	bool RendererD3D12::Initialize(IGameWindow* gameWindow, const RendererConfig& config)
	{
		if (mDeviceState >= ERenderDeviceState::DEVICE_INITIALIZED)
		{
			return true;
		}
		unsigned width, height;
		WindowsWindow* window = dynamic_cast<WindowsWindow*>(gameWindow);
		HWND hWnd = window->GetWindowHandle();
		gameWindow->GetWindowSize(width, height);
		if(!CreateDevice(hWnd, width, height, config))
		{
			return false;
		}
		mFrameBufferCount = config.frameBufferCount;
		// check MSAA setting
		if(config.sampleCount > 4 && mGpuInfo.msaa8QualityLevel > 0)
		{
			mSampleCount = 8;
			mSampleQuality = mGpuInfo.msaa8QualityLevel - 1;
		}
		else if(config.sampleCount > 1 && mGpuInfo.msaa4QualityLevel > 0)
		{
			mSampleCount = 4;
			mSampleQuality = mGpuInfo.msaa4QualityLevel - 1;
		}
		else
		{
			mSampleCount = 1;
			mSampleQuality = 0;
		}

		// check render target color format
		switch(config.colorFormat)
		{
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			mColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TinyImageFormat_B8G8R8A8_UNORM:
		case TinyImageFormat_B8G8R8A8_SRGB:
			mColorFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		default:
			mColorFormat = DXGI_FORMAT_UNKNOWN;
			break;
		}

		// check depth/stencil buffer format
		switch(config.depthStencilFormat)
		{
		case TinyImageFormat_D16_UNORM:
			mDepthFormat = DXGI_FORMAT_D16_UNORM;
			break;
		case TinyImageFormat_D24_UNORM_S8_UINT:
			mDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case TinyImageFormat_D32_SFLOAT:
			mDepthFormat = DXGI_FORMAT_D32_FLOAT;
			break;
		default:
			mDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		}

		// create command queue
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = 0;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;
		if(FAILED(mpDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mpCommandQueue))))
		{
			LogError("Fail to create default command queue.");
			return false;
		}

		// create default command list
		mppCommandAllocators = (ID3D12CommandAllocator**)tf_calloc(mFrameBufferCount, sizeof(ID3D12CommandAllocator*));
		memset(mppCommandAllocators, 0, sizeof(ID3D12CommandAllocator*) * mFrameBufferCount);
		// mpCommandFences = (D3D12Fence*)tf_calloc_memalign(mFrameBufferCount, alignof(D3D12Fence), sizeof(D3D12Fence));
		for(uint8_t i = 0; i < mFrameBufferCount; ++i)
		{
			if(FAILED(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mppCommandAllocators[i]))))
			{
				LogError("Fail to create command allocator %d", i);
				return false;
			}
			// Ensure(CreateFence(mpCommandFences[i]));
		}
		if(FAILED(mpDevice->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
			mppCommandAllocators[0], nullptr, 
			IID_PPV_ARGS(&mpCommandList)
		))) 
		{
			LogError("Fail to create default command list");
			return false;
		}
		// mpCommandList->Close();
		if(FAILED((mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpFrameFence)))))
		{
			LogError("Fail to crate frame fence");
			return false;
		}
		mFrameFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(mFrameFenceEvent == NULL)
		{
			LogError("Fail to crate event for fence");
			return false;
		}
		mpFrameFenceValue = (uint64_t*)tf_calloc(mFrameBufferCount, sizeof(uint64_t));
		memset(mpFrameFenceValue, 0, sizeof(uint64_t) * mFrameBufferCount);

		// create swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
			width, height, mColorFormat, FALSE,
			{ mSampleCount, mSampleQuality },
			DXGI_USAGE_RENDER_TARGET_OUTPUT,
			mFrameBufferCount,
			DXGI_SCALING_STRETCH,
			DXGI_SWAP_EFFECT_FLIP_DISCARD,
			DXGI_ALPHA_MODE_UNSPECIFIED,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		IDXGISwapChain1* swapChain1;
		if(FAILED(mpDXGIFactory->CreateSwapChainForHwnd(
			mpCommandQueue,
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		)))
		{
			LogError("Fail to create swap chain.");
			return false;
		}
		if(FAILED((swapChain1->QueryInterface(IID_PPV_ARGS(&mpSwapChain)))))
		{
			LogError("Fail to query interface IDXGISwapChain4.");
			swapChain1->Release();
			return false;
		}
		swapChain1->Release();

		if(FAILED(mpDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)))
		{
			LogWarning("Fail to MakeWindowAssociation.");
		}

		// create RTV/DSV heap for swap chain
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeap = {
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			mFrameBufferCount,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0,
		};
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeap = {
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0,
		};
		if(FAILED(mpDevice->CreateDescriptorHeap(&rtvHeap, IID_PPV_ARGS(&mpRtvHeap))))
		{
			LogError("Fail to create RTV heap");
			return false;
		}
		if(FAILED(mpDevice->CreateDescriptorHeap(&dsvHeap, IID_PPV_ARGS(&mpDsvHeap))))
		{
			LogError("Fail to create DSV heap");
			return false;
		}

		// create render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart());
		const unsigned descriptorSize = mDescriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
		ID3D12Resource* buffer = nullptr;
		mppSwapChainBuffers = (ID3D12Resource**)tf_calloc(mFrameBufferCount, sizeof(ID3D12Resource*));
		for (int i = 0; i < mFrameBufferCount; ++i)
		{
			mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));
			mpDevice->CreateRenderTargetView(buffer, nullptr, rtvHandle);
			mppSwapChainBuffers[i] = buffer;
			rtvHandle.Offset(descriptorSize);
		}

		// create depth/stencil buffer
		D3D12_RESOURCE_DESC depthDesc = {
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			width, height,
			1, 1,
			mDepthFormat,
			{mSampleCount, mSampleQuality},
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		};
		D3D12_CLEAR_VALUE clearDepth;
		clearDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearDepth.DepthStencil.Depth = 1.0f;
		clearDepth.DepthStencil.Stencil = 0;
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		if(FAILED(mpDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE, 
			&depthDesc, 
			D3D12_RESOURCE_STATE_COMMON, 
			&clearDepth, 
			IID_PPV_ARGS(&mpDepthBuffer)
		)))
		{
			LogError("Fail to create depth/stencil buffer");
			return false;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {
			mDepthFormat,
			D3D12_DSV_DIMENSION_TEXTURE2D,
			D3D12_DSV_FLAG_NONE,
		};
		dsvDesc.Texture2D.MipSlice = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mpDsvHeap->GetCPUDescriptorHandleForHeapStart());
		mpDevice->CreateDepthStencilView(mpDepthBuffer, &dsvDesc, dsvHandle);

		// execute init commands
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mpDepthBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		mpCommandList->ResourceBarrier(1, &barrier);
		mpCommandList->Close();

		ID3D12CommandList* const commandLists[] = { mpCommandList };
		mpCommandQueue->ExecuteCommandLists(1, commandLists);
		mFrameIndex += 1;
		mFrameBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
		mpCommandQueue->Signal(mpFrameFence, mFrameIndex);
		mpFrameFenceValue[mFrameBufferIndex] = mFrameIndex;

		mDeviceState = ERenderDeviceState::DEVICE_INITIALIZED;
		return true;
	}

	void RendererD3D12::Release()
	{
		if(mDeviceState >= ERenderDeviceState::DEVICE_RELEASED || mDeviceState < ERenderDeviceState::DEVICE_INITIALIZED)
		{
			return;
		}
		mDeviceState = ERenderDeviceState::DEVICE_RELEASED;

		if (mppCommandAllocators)
		{
			for (uint8_t i = 0; i < mFrameBufferCount; ++i)
			{
				mppCommandAllocators[i]->Release();
			}
			tf_free(mppCommandAllocators);
			mppCommandAllocators = nullptr;
		}

		if (mppSwapChainBuffers)
		{
			for (int i = 0; i < mFrameBufferCount; ++i)
			{
				ID3D12Resource* buff = mppSwapChainBuffers[i];
				SAFE_RELEASE(buff);
			}
			tf_free(mppSwapChainBuffers);
			mppSwapChainBuffers = nullptr;
		}
		SAFE_RELEASE(mpDepthBuffer);
		SAFE_RELEASE(mpSwapChain);
		SAFE_RELEASE(mpRtvHeap);
		SAFE_RELEASE(mpDsvHeap);

		SAFE_RELEASE(mpCommandQueue);
		SAFE_RELEASE(mpCommandList);
		SAFE_RELEASE(mpFrameFence);
		SAFE_CLOSE_HANDLE(mFrameFenceEvent);
		if(mpFrameFenceValue)
		{
			tf_free(mpFrameFenceValue);
			mpFrameFenceValue = nullptr;
		}

		SAFE_RELEASE(mpAdapter);
		SAFE_RELEASE(mpDXGIFactory);

		if(mpDeviceInfoQueue)
		{
			mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			SAFE_RELEASE(mpDeviceInfoQueue);
		}
		SAFE_RELEASE(mpDebug);
		SAFE_RELEASE(mpDevice);

		ReportLiveObjects(L"Check leaks on shutdown");
	}

	void RendererD3D12::Close()
	{
		if(mDeviceState == ERenderDeviceState::DEVICE_INITIALIZED)
		{
			WaitForComplete(mFrameIndex);
			mDeviceState = ERenderDeviceState::DEVICE_CLOSED;
		}
	}

	const GpuInfo& RendererD3D12::GetGpuInfo(int index)
	{
		return mGpuInfo;
	}

	void RendererD3D12::BeginFrame()
	{
		mFrameIndex += 1;
		auto frameFenceValue = mpFrameFenceValue[mFrameBufferIndex];
		WaitForComplete(frameFenceValue);

		ID3D12CommandAllocator* pAllocator = mppCommandAllocators[mFrameBufferIndex];
		pAllocator->Reset();
		mpCommandList->Reset(pAllocator, nullptr);

		auto frameBuffer = mppSwapChainBuffers[mFrameBufferIndex];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			frameBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mpCommandList->ResourceBarrier(1, &barrier);
		
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mpRtvHeap->GetCPUDescriptorHandleForHeapStart(), mFrameBufferIndex, mDescriptorSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
		mpCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(mpDsvHeap->GetCPUDescriptorHandleForHeapStart());
		mpCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}

	void RendererD3D12::EndFrame()
	{
	}

	void RendererD3D12::Present()
	{
		auto frameBuffer = mppSwapChainBuffers[mFrameBufferIndex];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			frameBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mpCommandList->ResourceBarrier(1, &barrier);
		mpCommandList->Close();

		ID3D12CommandList* const commandLists[] = { mpCommandList };
		mpCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
		mpFrameFenceValue[mFrameBufferIndex] = mFrameIndex;
		EnsureHResult(mpCommandQueue->Signal(mpFrameFence, mFrameIndex));
		EnsureHResult(mpSwapChain->Present(0, 0));
		mFrameBufferIndex = (mFrameBufferIndex + 1) % mFrameBufferCount;
	}

	void RendererD3D12::Resize()
	{

	}

	bool RendererD3D12::CreateDevice(HWND hWnd, uint32_t width, uint32_t height, const RendererConfig& config)
	{
		if(mpDevice)
		{
			return true;
		}

		mWindowHandle = hWnd;

		if(config.enableDebug)
		{
			EnableDebugLayer();
		}

		D3D_FEATURE_LEVEL featureLevels[4] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		
		// query for hardware adapter
		UINT createFactoryFlags = 0;
		if(config.enableDebug)
		{
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		}
		EnsureHResult(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&mpDXGIFactory)));

		int gpuCount = 0;
		IDXGIAdapter4* adapter;
		for (UINT i = 0; mpDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapterDesc;
			adapter->GetDesc3(&adapterDesc);

			if(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			{
				adapter->Release();
				continue;
			}

			for (const D3D_FEATURE_LEVEL level: featureLevels)
			{
				if(SUCCEEDED(D3D12CreateDevice(adapter, level, __uuidof(ID3D12Device), nullptr)))
				{
					IDXGIAdapter4* gpuInterface;
					if(SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&gpuInterface))))
					{
						gpuCount += 1;
						SAFE_RELEASE(gpuInterface);
						break;
					}
				}
			}
			adapter->Release();
		}

		if(gpuCount < 1)
		{
			LogError("Can't find Direct3D12 device.");
			return false;
		}

		D3D12GpuInfo* gpuInfoList = (D3D12GpuInfo*)alloca(sizeof(D3D12GpuInfo) * gpuCount);
		memset(gpuInfoList, 0, sizeof(D3D12GpuInfo) * gpuCount);

		struct GpuHandle
		{
			IDXGIAdapter4* adpater;
			ID3D12Device2* device;
		};
		GpuHandle* gpuList = (GpuHandle*)alloca(sizeof(GpuHandle) * gpuCount);
		memset(gpuList, 0, sizeof(GpuHandle) * gpuCount);

		gpuCount = 0;
		for (UINT i = 0; mpDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapterDesc;
			adapter->GetDesc3(&adapterDesc);

			if(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			{
				adapter->Release();
				continue;
			}
			for (const D3D_FEATURE_LEVEL level: featureLevels)
			{
				if(FAILED(D3D12CreateDevice(adapter, level, __uuidof(ID3D12Device), nullptr)))
				{
					continue;
				}

				if(FAILED(adapter->QueryInterface(IID_PPV_ARGS(&gpuList[gpuCount].adpater))))
				{
					continue;
				}

				ID3D12Device2* pDevice;
				D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&pDevice));
				gpuList[gpuCount].device = pDevice;
				auto &gpu = gpuInfoList[gpuCount];
				gpuCount += 1;

				gpu.featureLevel = level;
				wcscpy_s(gpu.vendorName, GPU_VENDOR_NAME_SIZE, adapterDesc.Description);
				gpu.vendorId = adapterDesc.VendorId;
				gpu.deviceId = adapterDesc.DeviceId;
				gpu.revision = adapterDesc.Revision;
				gpu.videoMemory = adapterDesc.DedicatedVideoMemory;
				gpu.sharedMemory = adapterDesc.SharedSystemMemory;

				pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &gpu.featureData, sizeof(gpu.featureData));
				pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &gpu.featureData1, sizeof(gpu.featureData1));
				break;
			}
			adapter->Release();
		}

		// select the best one
		int gpuIndex = 0;
		for(int i=1; i<gpuCount; ++i)
		{
			auto &gpu1 = gpuInfoList[gpuIndex];
			auto &gpu2 = gpuInfoList[i];
			if(gpu2.featureData1.WaveOps != gpu1.featureData1.WaveOps)
			{
				if(gpu2.featureData1.WaveOps)
				{
					gpuIndex = i;
				}
				continue;
			}
			if(gpu2.featureLevel != gpu1.featureLevel)
			{
				if(gpu2.featureLevel > gpu1.featureLevel)
				{
					gpuIndex = i;
				}
				continue;
			}
			if(gpu2.videoMemory > gpu1.videoMemory)
			{
				gpuIndex = i;
			}
		}

		mpAdapter = gpuList[gpuIndex].adpater;
		mpDevice = gpuList[gpuIndex].device;
		memcpy(&mGpuInfo, &gpuInfoList[gpuIndex], sizeof(GpuInfo));

		for(int i=0; i<gpuCount; ++i)
		{
			if(i != gpuIndex)
			{
				SAFE_RELEASE(gpuList[i].adpater);
				SAFE_RELEASE(gpuList[i].device);
			}
		}

		// check MSAA support
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;
		if(SUCCEEDED(mpDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels))))
		{
			mGpuInfo.msaa4QualityLevel = (int) msaaQualityLevels.NumQualityLevels;
		}
		msaaQualityLevels.SampleCount = 8;
		msaaQualityLevels.NumQualityLevels = 0;
		if(SUCCEEDED(mpDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels))))
		{
			mGpuInfo.msaa8QualityLevel = (int) msaaQualityLevels.NumQualityLevels;
		}

		LogInfo("Device: %ls", mGpuInfo.vendorName);
		LogInfo("Vendor ID: %d", mGpuInfo.vendorId);
		LogInfo("Revision ID: %d", mGpuInfo.revision);
		LogInfo("Video Memory: %zu", mGpuInfo.videoMemory);
		LogInfo("MSAA 4x: %d", mGpuInfo.msaa4QualityLevel);
		LogInfo("MSAA 8x: %d", mGpuInfo.msaa8QualityLevel);

		for(int i=0; i<D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			mDescriptorSize[i] = mpDevice->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)i);
		}

		if(config.enableDebug)
		{
			if (SUCCEEDED(mpDevice->QueryInterface(IID_PPV_ARGS(&mpDeviceInfoQueue))))
			{
				mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			}

			// Suppress whole categories of messages
			// D3D12_MESSAGE_CATEGORY Categories[] = 
			// {
			// };

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER infoFilter = {};
			// 		NewinfoFilterFilter.DenyList.NumCategories = _countof(Categories);
			// 		infoFilter.DenyList.pCategoryList = Categories;
			infoFilter.DenyList.NumSeverities = _countof(Severities);
			infoFilter.DenyList.pSeverityList = Severities;
			infoFilter.DenyList.NumIDs = _countof(DenyIds);
			infoFilter.DenyList.pIDList = DenyIds;

			CheckAndReturnFalse(mpDeviceInfoQueue->PushStorageFilter(&infoFilter));
		} // enable debug checks
		
		return true;
	}

	void RendererD3D12::ReportLiveObjects(const wchar_t* prompt)
	{
#ifdef _DEBUG
		if(prompt)
		{
			OutputDebugStringW(L"[");
			OutputDebugStringW(prompt);
			OutputDebugStringW(L"]\n");
		}
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
#endif
	}

	void RendererD3D12::WaitForComplete(uint64_t frameIndex)
	{
		if(mpFrameFence->GetCompletedValue() < frameIndex)
		{
			mpFrameFence->SetEventOnCompletion(frameIndex, mFrameFenceEvent);
			WaitForSingleObject(mFrameFenceEvent, INFINITE);
		}
	}

	static D3D12_COMMAND_LIST_TYPE D3D12CommandListTypeConverter[ECommandType::MaxCount] = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY
	};

	void RendererD3D12::EnableDebugLayer()
	{
		if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&mpDebug))))
		{
			mpDebug->EnableDebugLayer();
			ID3D12Debug1* pDebug1 = NULL;
			if (SUCCEEDED(mpDebug->QueryInterface(IID_PPV_ARGS(&pDebug1))))
			{
				pDebug1->SetEnableGPUBasedValidation(TRUE);
				pDebug1->Release();
			}
		}
	}
} // namespace wyc
