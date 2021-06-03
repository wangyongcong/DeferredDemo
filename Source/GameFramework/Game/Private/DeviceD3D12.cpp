#include "GameFrameworkPCH.h"
#include "DeviceD3D12.h"
#include <chrono>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include "AssertionMacros.h"
#include "LogMacros.h"
#include "WindowsGameWindow.h"
#include "D3DHelper.h"


namespace wyc
{
	RenderDeviceD3D12::RenderDeviceD3D12()
		: mInitialized(false)
		, mFrameBuffCount(3)
		, mFenceValue(0)
		// D3D12 device data
		, mpDebug(nullptr)
		, mpDXGIFactory(nullptr)
		, mpAdapter(nullptr)
		, mpDevice(nullptr)
	{
	}

	RenderDeviceD3D12::~RenderDeviceD3D12()
	{
		if(mBackBuffers)
		{
			delete[] mBackBuffers;
			mBackBuffers = nullptr;
		}
		if(mCommandAllocators)
		{
			delete[] mCommandAllocators;
			mCommandAllocators = nullptr;
		}
		if(mFrameFenceValues)
		{
			delete[] mFrameFenceValues;
			mFrameFenceValues = nullptr;
		}
		SAFE_RELEASE(mpDXGIFactory);
		SAFE_RELEASE(mpDevice);
		SAFE_RELEASE(mpDebug);
	}

	bool RenderDeviceD3D12::Initialzie(IGameWindow* gameWindow)
	{
		if (mInitialized)
		{
			return true;
		}
		unsigned width, height;
		WindowsGameWindow* win = dynamic_cast<WindowsGameWindow*>(gameWindow);
		HWND hWnd = win->GetWindowHandle();
		gameWindow->GetWindowSize(width, height);
		if (!CreateDevice(hWnd, width, height))
		{
			return false;
		}

		// Create command queue
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;
		CheckAndReturnFalse(mpDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mCommandQueue)));

		// Create swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = mFrameBuffCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		CheckAndReturnFalse(mpDXGIFactory->CreateSwapChainForHwnd(
			mCommandQueue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		));

		CheckAndReturnFalse(mpDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
		CheckAndReturnFalse(swapChain1.As(&mSwapChain));
		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		// create RTV heap for swap chain
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = mFrameBuffCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		CheckAndReturnFalse(mpDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mSwapChainHeap)));

		// create render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mSwapChainHeap->GetCPUDescriptorHandleForHeapStart());
		mDescriptorSizeRTV = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		mBackBuffers = new ComPtr<ID3D12Resource>[mFrameBuffCount];
		for (int i = 0; i < mFrameBuffCount; ++i)
		{
			ComPtr<ID3D12Resource> backBuffer;
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			mpDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
			mBackBuffers[i] = backBuffer;
			rtvHandle.Offset(mDescriptorSizeRTV);
		}

		// create command allocator & command list
		mCommandAllocators = new ComPtr<ID3D12CommandAllocator>[mFrameBuffCount];

		for (int i = 0; i < mFrameBuffCount; ++i)
		{
			ComPtr<ID3D12CommandAllocator> commandAllocator;
			mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
			mCommandAllocators[i] = commandAllocator;
		}
		mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[mCurrentBackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&mCommandList));
		mCommandList->Close();

		mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
		mFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		mFrameFenceValues = new uint64_t[mFrameBuffCount];

		mInitialized = true;
		return true;
	}

	void RenderDeviceD3D12::Render()
	{
		auto commandAllocator = mCommandAllocators[mCurrentBackBufferIndex];
		auto backBuffer = mBackBuffers[mCurrentBackBufferIndex];

		commandAllocator->Reset();
		mCommandList->Reset(commandAllocator.Get(), nullptr);

		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				backBuffer.Get(), 
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			mCommandList->ResourceBarrier(1, &barrier);
			float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mSwapChainHeap->GetCPUDescriptorHandleForHeapStart(), mCurrentBackBufferIndex, mDescriptorSizeRTV);
			mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		}

		SwapBuffer();
	}

	void RenderDeviceD3D12::Close()
	{
		Signal();
		WaitForFence();
		CloseHandle(mFenceEvent);
	}

	bool RenderDeviceD3D12::CreateSwapChain(const SSwapChainDesc& Desc)
	{
		return true;
	}

	void RenderDeviceD3D12::SwapBuffer()
	{
		auto backBuffer = mBackBuffers[mCurrentBackBufferIndex];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		mCommandList->ResourceBarrier(1, &barrier);

		mCommandList->Close();

		ID3D12CommandList* const commandLists[] = {
			mCommandList.Get()
		};
		mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		Signal();

		CHECK_HRESULT(mSwapChain->Present(1, 0));
		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		WaitForFence();
	}

	bool RenderDeviceD3D12::CreateDevice(HWND hWnd, uint32_t width, uint32_t height)
	{
		if(mpDevice)
		{
			return true;
		}

#if defined(_DEBUG)
		EnableDebugLayer();
#endif

		D3D_FEATURE_LEVEL featureLevels[4] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		
		// query for hardware adapter
		UINT createFactoryFlags = 0;
#ifdef _DEBUG
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		CHECK_HRESULT(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&mpDXGIFactory)));
		IDXGIAdapter4* adapter;
		for (UINT i = 0; mpDXGIFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND && !mpDevice; ++i)
		{
			DXGI_ADAPTER_DESC3 adapterDesc;
			adapter->GetDesc3(&adapterDesc);

			if(!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
			{
				constexpr int count = sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL);
				for (auto level: featureLevels)
				{
					if(FAILED(D3D12CreateDevice(adapter, level, __uuidof(ID3D12Device), nullptr)))
					{
						continue;
					}
					if(FAILED(adapter->QueryInterface(IID_PPV_ARGS(&mpAdapter))))
					{
						continue;
					}
					D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&mpDevice));
					GpuInfo.featureLevel = level;
					mpDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &GpuInfo.featureData, sizeof(GpuInfo.featureData));
					mpDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &GpuInfo.featureData1, sizeof(GpuInfo.featureData1));
					GpuInfo.DedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
					GpuInfo.VendorId = adapterDesc.VendorId;
					GpuInfo.DeviceId = adapterDesc.DeviceId;
					GpuInfo.Revision = adapterDesc.Revision;
					GpuInfo.Name = adapterDesc.Description;
					break;
				}
			}
			adapter->Release();
		}
		if(!mpDevice)
		{
			return false;
		}

		LogInfo("Device: %s", GpuInfo.Name);

#ifdef _DEBUG
		ComPtr<ID3D12InfoQueue> deviceInfoQueue;
		if (SUCCEEDED(mpDevice->QueryInterface(IID_PPV_ARGS(&deviceInfoQueue))))
		{
			deviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			deviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			deviceInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
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

		CheckAndReturnFalse(deviceInfoQueue->PushStorageFilter(&infoFilter));
#endif // _DEBUG
		
		return true;
	}

	void RenderDeviceD3D12::EnableDebugLayer()
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

	void RenderDeviceD3D12::Signal()
	{
		uint64_t fenceValueForSignal = ++mFenceValue;
		mFrameFenceValues[mCurrentBackBufferIndex] = fenceValueForSignal;
		CHECK_HRESULT(mCommandQueue->Signal(mFence.Get(), mFenceValue));

	}

	void RenderDeviceD3D12::WaitForFence()
	{
		if (mFence->GetCompletedValue() < mFenceValue)
		{
			mFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
			DWORD duration = (DWORD)(std::chrono::milliseconds::max().count());
			::WaitForSingleObject(mFenceEvent, duration);
		}
	}

} // namespace wyc
