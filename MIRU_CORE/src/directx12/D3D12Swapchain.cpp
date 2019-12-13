#include "common.h"
#include "D3D12Swapchain.h"
#include "D3D12Context.h"

using namespace miru;
using namespace d3d12;

Swapchain::Swapchain(CreateInfo* pCreateInfo)
	:m_Factory(std::dynamic_pointer_cast<Context>(pCreateInfo->pContext)->m_Factory),
	m_Device(std::dynamic_pointer_cast<Context>(pCreateInfo->pContext)->m_Device),
	m_Swapchain(nullptr)
{
	m_CI = *pCreateInfo;
	ID3D12CommandQueue* cmdQueue = std::dynamic_pointer_cast<Context>(m_CI.pContext)->m_Queues[0];

	//Create Swapchain
	m_SwapchainDesc.Width = m_CI.width;
	m_SwapchainDesc.Height = m_CI.height;
	m_SwapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_SwapchainDesc.Stereo = false;
	m_SwapchainDesc.SampleDesc = { 1, 0 };
	m_SwapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapchainDesc.BufferCount = m_CI.swapchainCount;	//One of these is locked by Windows
	m_SwapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	m_SwapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	m_SwapchainDesc.Flags = m_CI.vSync == false ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	IDXGISwapChain1* swapchain;
	MIRU_ASSERT(m_Factory->CreateSwapChainForHwnd(cmdQueue, static_cast<HWND>(m_CI.pWindow), &m_SwapchainDesc, nullptr, nullptr, &swapchain), "ERROR: D3D12: Failed to create Swapchain.");
	m_Swapchain = reinterpret_cast<IDXGISwapChain4*>(swapchain);
	D3D12SetName(m_Swapchain, m_CI.debugName);

	m_Width= m_CI.width;
	m_Height = m_CI.height;
	m_Format = m_SwapchainDesc.Format;

	//Create Swapchian RTV
	m_SwapchainRTVDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_SwapchainRTVDescHeapDesc.NumDescriptors = m_SwapchainDesc.BufferCount;
	m_SwapchainRTVDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_SwapchainRTVDescHeapDesc.NodeMask = 0;
	MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_SwapchainRTVDescHeapDesc, IID_PPV_ARGS(&m_SwapchainRTVDescHeap)), "ERROR: D3D12: Failed to get Swapchain Images");
	D3D12SetName(m_SwapchainRTVDescHeap, (std::string(m_CI.debugName) + ": RTV Descriptor Heap").c_str());

	D3D12_CPU_DESCRIPTOR_HANDLE swapchainRTV_CPUDescHandle = m_SwapchainRTVDescHeap->GetCPUDescriptorHandleForHeapStart();
	size_t rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_SwapchainRTVs.reserve(m_SwapchainRTVDescHeapDesc.NumDescriptors);
	for(UINT i = 0; i < m_SwapchainRTVDescHeapDesc.NumDescriptors; i++)
	{
		ID3D12Resource* swapchainRTV;
		MIRU_ASSERT(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainRTV)), "ERROR: D3D12: Failed to get Swapchain Images");
		m_Device->CreateRenderTargetView(swapchainRTV, nullptr, swapchainRTV_CPUDescHandle);
		D3D12SetName(swapchainRTV, (std::string(m_CI.debugName) + ": RTV " + std::to_string(i)).c_str());

		m_SwapchainRTVs.push_back(swapchainRTV);
		swapchainRTV_CPUDescHandle.ptr += rtvDescriptorSize;
	}
}

Swapchain::~Swapchain()
{
	for (auto& swapchainRTV : m_SwapchainRTVs)
		SAFE_RELEASE(swapchainRTV);

	SAFE_RELEASE(m_SwapchainRTVDescHeap);
	SAFE_RELEASE(m_Swapchain);
}

void Swapchain::Resize(uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;

	//Resize Buffers and release old resources
	m_Swapchain->ResizeBuffers(m_CI.swapchainCount, width, height, m_SwapchainDesc.Format, m_SwapchainDesc.Flags);

	for (auto& swapchainRTV : m_SwapchainRTVs)
		SAFE_RELEASE(swapchainRTV);

	SAFE_RELEASE(m_SwapchainRTVDescHeap);

	//Create Swapchian RTV
	m_SwapchainRTVDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_SwapchainRTVDescHeapDesc.NumDescriptors = m_SwapchainDesc.BufferCount;
	m_SwapchainRTVDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_SwapchainRTVDescHeapDesc.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&m_SwapchainRTVDescHeapDesc, IID_PPV_ARGS(&m_SwapchainRTVDescHeap));
	D3D12SetName(m_SwapchainRTVDescHeap, (std::string(m_CI.debugName) + ": RTV Descriptor Heap").c_str());

	D3D12_CPU_DESCRIPTOR_HANDLE swapchainRTV_CPUDescHandle = m_SwapchainRTVDescHeap->GetCPUDescriptorHandleForHeapStart();
	size_t rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_SwapchainRTVs.reserve(m_SwapchainRTVDescHeapDesc.NumDescriptors);
	for (UINT i = 0; i < m_SwapchainRTVDescHeapDesc.NumDescriptors; i++)
	{
		ID3D12Resource* swapchainRTV;
		m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainRTV));
		m_Device->CreateRenderTargetView(swapchainRTV, nullptr, swapchainRTV_CPUDescHandle);
		D3D12SetName(swapchainRTV, (std::string(m_CI.debugName) + ": RTV " + std::to_string(i)).c_str());

		m_SwapchainRTVs.push_back(swapchainRTV);
		swapchainRTV_CPUDescHandle.ptr += rtvDescriptorSize;
	}
}
