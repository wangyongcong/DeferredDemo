#include "RenderDevice.h"
#include <chrono>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include "GameWindow.h"
#include "AssertionMacros.h"
#include "LogMacros.h"


namespace wyc
{
	CRenderDeviceD3D12::CRenderDeviceD3D12()
		: mInitialized(false)
		, mFrameBuffCount(3)
		, mFenceValue(0)
	{
	}

	CRenderDeviceD3D12::~CRenderDeviceD3D12()
	{

	}

	bool CRenderDeviceD3D12::Initialzie(CGameWindow* gameWindow)
	{
		if (mInitialized)
		{
			return true;
		}
		HWND hWnd = gameWindow->GetWindowHandle();
		unsigned width, height;
		gameWindow->GetWindowSize(width, height);
		if (!CreateDevice(hWnd, width, height))
		{
			return false;
		}
		mInitialized = true;
		return true;
	}

	void CRenderDeviceD3D12::Render()
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

		{
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

			uint64_t fenceValueForSignal = ++mFenceValue;
			mFrameFenceValues[mCurrentBackBufferIndex] = fenceValueForSignal;
			ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mFenceValue));

			ThrowIfFailed(mSwapChain->Present(1, 0));

			mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

			if (mFence->GetCompletedValue() < mFenceValue)
			{
				mFence->SetEventOnCompletion(mFenceValue, hFenceEvent);
				DWORD duration = (DWORD)(std::chrono::milliseconds::max().count());
				::WaitForSingleObject(hFenceEvent, duration);
			}
		}
	}

	bool CRenderDeviceD3D12::CreateDevice(HWND hWnd, uint32_t width, uint32_t height)
	{
#if defined(_DEBUG)
		// enable DEBUG layer
		ComPtr<ID3D12Debug> debugInterface;
		ReturnFalseIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
		debugInterface->EnableDebugLayer();
#endif
		
		// query for hardware adapter
		ComPtr<IDXGIFactory4> dxgiFactory4;
		UINT createFactoryFlags = 0;
#ifdef _DEBUG
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		ReturnFalseIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));
		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; dxgiFactory4->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0
				&& SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))
				&& dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				ReturnFalseIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
		if (dxgiAdapter4.Get() == NULL)
		{
			return false;
		}
		
		// create D3D device
		ReturnFalseIfFailed(D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));

#ifdef _DEBUG
		ComPtr<ID3D12InfoQueue> deviceInfoQueue;
		if (SUCCEEDED(mDevice.As(&deviceInfoQueue)))
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

		ReturnFalseIfFailed(deviceInfoQueue->PushStorageFilter(&infoFilter));
#endif // _DEBUG

		// Create command queue
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;
		ReturnFalseIfFailed(mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mCommandQueue)));

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
		ReturnFalseIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
			mCommandQueue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		));

		ReturnFalseIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
		ReturnFalseIfFailed(swapChain1.As(&mSwapChain));
		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		// create RTV heap for swap chain
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = mFrameBuffCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		ReturnFalseIfFailed(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mSwapChainHeap)));

		// create render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mSwapChainHeap->GetCPUDescriptorHandleForHeapStart());
		mDescriptorSizeRTV = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		mBackBuffers = new ComPtr<ID3D12Resource>[mFrameBuffCount];
		for (int i = 0; i < mFrameBuffCount; ++i)
		{
			ComPtr<ID3D12Resource> backBuffer;
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
			mBackBuffers[i] = backBuffer;
			rtvHandle.Offset(mDescriptorSizeRTV);
		}

		// create command allocator & command list
		mCommandAllocators = new ComPtr<ID3D12CommandAllocator>[mFrameBuffCount];

		for (int i = 0; i < mFrameBuffCount; ++i)
		{
			ComPtr<ID3D12CommandAllocator> commandAllocator;
			mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
			mCommandAllocators[i] = commandAllocator;
		}
		mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[mCurrentBackBufferIndex].Get(), nullptr, IID_PPV_ARGS(&mCommandList));
		mCommandList->Close();

		mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
		hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		mFrameFenceValues = new uint64_t[mFrameBuffCount];

		return true;
	}

} // namespace wyc
