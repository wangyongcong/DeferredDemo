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
		, mMaxFrameLatency(3)
		, mFrameCount(0)
		, mFrameIndex(0)
		, mBackBufferIndex(0)
		, mDescriptorSizeRTV(0)
		// D3D12 device data
		, mpDebug(nullptr)
		, mpDXGIFactory(nullptr)
		, mpAdapter(nullptr)
		, mpDevice(nullptr)
		, mpDeviceInfoQueue(nullptr)
		, mpCommandQueue(nullptr)
		, mpCommandList(nullptr)
		, mppCommandAllocators(nullptr)
		, mpCommandFences(nullptr)
		, mpSwapChain(nullptr)
		, mpSwapChainHeap(nullptr)
		, mppBackBuffers(nullptr)
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
		const WindowsWindow* window = dynamic_cast<WindowsWindow*>(gameWindow);
		const HWND hWnd = window->GetWindowHandle();
		gameWindow->GetWindowSize(width, height);
		if(!CreateDevice(hWnd, width, height, config))
		{
			return false;
		}
		if (!CreateCommandQueue())
		{
			return false;
		}
		if (!CreateCommandList())
		{
			return false;
		}

		// Create swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = mMaxFrameLatency;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		IDXGISwapChain1* swapChain1;
		CheckAndReturnFalse(mpDXGIFactory->CreateSwapChainForHwnd(
			mpCommandQueue,
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		));

		CheckAndReturnFalse(mpDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
		CheckAndReturnFalse(swapChain1->QueryInterface(IID_PPV_ARGS(&mpSwapChain)));
		swapChain1->Release();
		mBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

		// create RTV heap for swap chain
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = mMaxFrameLatency;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		CheckAndReturnFalse(mpDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mpSwapChainHeap)));

		// create render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpSwapChainHeap->GetCPUDescriptorHandleForHeapStart());
		mDescriptorSizeRTV = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		mppBackBuffers = (ID3D12Resource**)tf_calloc(mMaxFrameLatency, sizeof(ID3D12Resource*));
		for (int i = 0; i < mMaxFrameLatency; ++i)
		{
			ID3D12Resource* backBuffer = nullptr;
			mpSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			mpDevice->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);
			mppBackBuffers[i] = backBuffer;
			rtvHandle.Offset(mDescriptorSizeRTV);
		}
		
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
			for (uint8_t i = 0; i < mMaxFrameLatency; ++i)
			{
				mppCommandAllocators[i]->Release();
			}
			tf_free(mppCommandAllocators);
			mppCommandAllocators = nullptr;
		}
		if (mpCommandFences)
		{
			for (uint8_t i = 0; i < mMaxFrameLatency; ++i)
			{
				ReleaseFence(mpCommandFences[i]);
			}
			tf_free(mpCommandFences);
			mpCommandFences = nullptr;
		}
		ReleaseFence(mQueueFence);

		if (mppBackBuffers)
		{
			for (int i = 0; i < mMaxFrameLatency; ++i)
			{
				ID3D12Resource* buff = mppBackBuffers[i];
				SAFE_RELEASE(buff);
			}
			tf_free(mppBackBuffers);
			mppBackBuffers = nullptr;
		}
		SAFE_RELEASE(mpSwapChain);
		SAFE_RELEASE(mpSwapChainHeap);

		SAFE_RELEASE(mpCommandList);
		SAFE_RELEASE(mpCommandQueue);

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
			mDeviceState = ERenderDeviceState::DEVICE_CLOSED;
			DeviceFence& fence = mpCommandFences[mFrameIndex];
			WaitForFence(fence);
		}
	}

	const GpuInfo& RendererD3D12::GetGpuInfo(int index)
	{
		return mGpuInfo;
	}

	bool RendererD3D12::CreateSwapChain(const SwapChainDesc& desc)
	{
		return true;
	}

	void RendererD3D12::BeginFrame()
	{
		mFrameIndex = mFrameCount % mMaxFrameLatency;
		ID3D12CommandAllocator* pAllocator = mppCommandAllocators[mFrameIndex];
		DeviceFence& fence = mpCommandFences[mFrameIndex];
		WaitForFence(fence);

		pAllocator->Reset();
		mpCommandList->Reset(pAllocator, nullptr);

		auto backBuffer = mppBackBuffers[mBackBufferIndex];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mpCommandList->ResourceBarrier(1, &barrier);
		
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mpSwapChainHeap->GetCPUDescriptorHandleForHeapStart(), mBackBufferIndex, mDescriptorSizeRTV);
		mpCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

	void RendererD3D12::Present()
	{
		auto backBuffer = mppBackBuffers[mBackBufferIndex];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mpCommandList->ResourceBarrier(1, &barrier);
		mpCommandList->Close();

		ID3D12CommandList* const commandLists[] = { mpCommandList };
		mpCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
		DeviceFence& fence = mpCommandFences[mFrameIndex];
		EnsureHResult(mpCommandQueue->Signal(fence.mpDxFence, ++fence.mFenceValue));

		EnsureHResult(mpSwapChain->Present(1, 0));
		mBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
		mFrameCount += 1;
	}

	bool RendererD3D12::CreateDevice(HWND hWnd, uint32_t width, uint32_t height, const RendererConfig& config)
	{
		if(mpDevice)
		{
			return true;
		}

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
			mGpuInfo.msaa4 = (int)msaaQualityLevels.NumQualityLevels;
		}
		msaaQualityLevels.SampleCount = 8;
		msaaQualityLevels.NumQualityLevels = 0;
		if(SUCCEEDED(mpDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels))))
		{
			mGpuInfo.msaa8 = (int)msaaQualityLevels.NumQualityLevels;
		}

		LogInfo("Device: %ls", mGpuInfo.vendorName);
		LogInfo("Vendor ID: %d", mGpuInfo.vendorId);
		LogInfo("Revision ID: %d", mGpuInfo.revision);
		LogInfo("Video Memory: %zu", mGpuInfo.videoMemory);
		LogInfo("MSAA 4x: %d", mGpuInfo.msaa4);
		LogInfo("MSAA 8x: %d", mGpuInfo.msaa8);

		if(config.enableDebug)
		{
			if (SUCCEEDED(mpDevice->QueryInterface(IID_PPV_ARGS(&mpDeviceInfoQueue))))
			{
				mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				mpDeviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			}

			// Suppress whole categories of messages
			// 		D3D12_MESSAGE_CATEGORY Categories[] = 
			// 		{
			// 		};

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

	bool RendererD3D12::CreateCommandQueue()
	{
		Ensure(mpDevice);
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;
		if(FAILED(mpDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mpCommandQueue))))
		{
			return false;
		}
		if(!CreateFence(mQueueFence))
		{
			return false;
		}
		return true;
	}

	bool RendererD3D12::CreateFence(DeviceFence& outFence)
	{
		Ensure(mpDevice);
		ID3D12Fence* pFence;
		if (FAILED(mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence))))
		{
			return false;
		}
		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(hEvent == NULL)
		{
			pFence->Release();
			return false;
		}
		outFence.mpDxFence = pFence;
		outFence.mhWaitEvent = hEvent;
		outFence.mFenceValue = 0;
		return true;
	}

	void RendererD3D12::ReleaseFence(DeviceFence& fence)
	{
		SAFE_RELEASE(fence.mpDxFence);
		SAFE_CLOSE_HANDLE(fence.mhWaitEvent);
	}

	void RendererD3D12::WaitForFence(DeviceFence& fence)
	{
		if(fence.mpDxFence->GetCompletedValue() < fence.mFenceValue)
		{
			fence.mpDxFence->SetEventOnCompletion(fence.mFenceValue, fence.mhWaitEvent);
			WaitForSingleObject(fence.mhWaitEvent, INFINITE);
		}
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

	bool RendererD3D12::CreateCommandList()
	{
		mppCommandAllocators = (ID3D12CommandAllocator**)tf_calloc(mMaxFrameLatency, sizeof(ID3D12CommandAllocator*));
		mpCommandFences = (DeviceFence*)tf_calloc_memalign(mMaxFrameLatency, alignof(DeviceFence), sizeof(DeviceFence));
		for(uint8_t i = 0; i < mMaxFrameLatency; ++i)
		{
			EnsureHResult(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mppCommandAllocators[i])));
			Ensure(CreateFence(mpCommandFences[i]));
		}
		EnsureHResult(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mppCommandAllocators[0], nullptr, IID_PPV_ARGS(&mpCommandList)));
		mpCommandList->Close();
		return true;
	}

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
