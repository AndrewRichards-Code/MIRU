#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Swapchain.h"
#include "D3D12Context.h"

using namespace miru;
using namespace d3d12;

Swapchain::Swapchain(CreateInfo* pCreateInfo)
	:m_Factory(ref_cast<Context>(pCreateInfo->context)->m_Factory),
	m_Device(ref_cast<Context>(pCreateInfo->context)->m_Device),
	m_Swapchain(nullptr)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	ID3D12CommandQueue* cmdQueue = ref_cast<Context>(m_CI.context)->m_Queues[0];

	//Create Swapchain
	std::pair<DXGI_FORMAT, DXGI_COLOR_SPACE_TYPE> surfaceFormat;
	switch (m_CI.bpcColourSpace)
	{
	default:
	case base::Swapchain::BPC_ColourSpace::B8G8R8A8_UNORM_SRGB_NONLINEAR:
		surfaceFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 }; break;
	case base::Swapchain::BPC_ColourSpace::A2B10G10R10_UNORM_PACK32_SRGB_NONLINEAR:
		surfaceFormat = { DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 }; break;
	case base::Swapchain::BPC_ColourSpace::A2B10G10R10_UNORM_PACK32_HDR10_ST2084:
		surfaceFormat = { DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 }; break;
	case base::Swapchain::BPC_ColourSpace::R16G16B16A16_SFLOAT_EXTENDED_SRGB_LINEAR:
		surfaceFormat = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 }; break;
	}

	m_SwapchainDesc.Width = m_CI.width;
	m_SwapchainDesc.Height = m_CI.height;
	m_SwapchainDesc.Format = surfaceFormat.first;
	m_SwapchainDesc.Stereo = false;
	m_SwapchainDesc.SampleDesc = { 1, 0 };
	m_SwapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapchainDesc.BufferCount = m_CI.swapchainCount;	//One of these is locked by Windows
	m_SwapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	m_SwapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	m_SwapchainDesc.Flags = m_CI.vSync == false ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	IDXGISwapChain1* swapchain;
	#if defined(MIRU_WIN64_UWP)
	MIRU_ASSERT(m_Factory->CreateSwapChainForCoreWindow(cmdQueue, reinterpret_cast<IUnknown*>(m_CI.pWindow), &m_SwapchainDesc, nullptr, &swapchain), "ERROR: D3D12: Failed to create Swapchain.");
	#else
	MIRU_ASSERT(m_Factory->CreateSwapChainForHwnd(cmdQueue, static_cast<HWND>(m_CI.pWindow), &m_SwapchainDesc, nullptr, nullptr, &swapchain), "ERROR: D3D12: Failed to create Swapchain.");
	#endif
	m_Swapchain = reinterpret_cast<IDXGISwapChain4*>(swapchain);

	//Set Colour space
	IDXGIOutput6* output;
	MIRU_ASSERT(m_Swapchain->GetContainingOutput(reinterpret_cast<IDXGIOutput**>(&output)), "ERROR: D3D12: Failed to get containing Output.");
	DXGI_OUTPUT_DESC1 outputDesc;
	MIRU_ASSERT(output->GetDesc1(&outputDesc), "ERROR: D3D12: Failed to get Output Description.");
	MIRU_D3D12_SAFE_RELEASE(output);

	if (outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) //HDR
	{
		if (surfaceFormat.second == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
		{
			UINT colourSpaceSupport;
			MIRU_ASSERT(m_Swapchain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &colourSpaceSupport), "ERROR: D3D12: Failed to check ColourSpace support.");
			if (arc::BitwiseCheck(static_cast<DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG>(colourSpaceSupport), DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
			{
				MIRU_ASSERT(m_Swapchain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020), "ERROR: D3D12: Failed to set ColourSpace.");
				surfaceFormat.second = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
			}
		}
	}
	else if (outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) //SDR
	{
		surfaceFormat.second = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	}
	else
	{
		MIRU_ASSERT(true, "ERROR: D3D12: Unknown colour space.")
	}
	m_Format = surfaceFormat.first;
	m_ColorSpace = surfaceFormat.second;

	//D3D12SetName(m_Swapchain, m_CI.debugName);
	MIRU_ASSERT(m_Swapchain->GetSourceSize(&m_Width, &m_Height), "ERROR: D3D12: Failed to get size of the Swapchain.");

	//Create Swapchian RTV
	m_SwapchainRTVDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_SwapchainRTVDescHeapDesc.NumDescriptors = m_SwapchainDesc.BufferCount;
	m_SwapchainRTVDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_SwapchainRTVDescHeapDesc.NodeMask = 0;
	MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_SwapchainRTVDescHeapDesc, IID_PPV_ARGS(&m_SwapchainRTVDescHeap)), "ERROR: D3D12: Failed to Create Swapchain Image Descriptor Heap.");
	D3D12SetName(m_SwapchainRTVDescHeap, m_CI.debugName + ": RTV Descriptor Heap");

	D3D12_CPU_DESCRIPTOR_HANDLE swapchainRTV_CPUDescHandle = m_SwapchainRTVDescHeap->GetCPUDescriptorHandleForHeapStart();
	size_t rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_SwapchainRTVs.reserve(m_SwapchainRTVDescHeapDesc.NumDescriptors);
	m_SwapchainRTV_CPU_Desc_Handles.reserve(m_SwapchainRTVDescHeapDesc.NumDescriptors);
	for(UINT i = 0; i < m_SwapchainRTVDescHeapDesc.NumDescriptors; i++)
	{
		ID3D12Resource* swapchainRTV;
		MIRU_ASSERT(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainRTV)), "ERROR: D3D12: Failed to get Swapchain Image.");
		m_Device->CreateRenderTargetView(swapchainRTV, nullptr, swapchainRTV_CPUDescHandle);
		D3D12SetName(swapchainRTV, m_CI.debugName + ": RTV " + std::to_string(i));

		m_SwapchainRTVs.push_back(swapchainRTV);
		m_SwapchainRTV_CPU_Desc_Handles.push_back(swapchainRTV_CPUDescHandle);
		swapchainRTV_CPUDescHandle.ptr += rtvDescriptorSize;
	}

	FillSwapchainImageAndViews((void**)m_SwapchainRTVs.data(), (void*)m_SwapchainRTV_CPU_Desc_Handles.data(), m_Width, m_Height, static_cast<uint32_t>(m_Format));
}

Swapchain::~Swapchain()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto& swapchainRTV : m_SwapchainRTVs)
		MIRU_D3D12_SAFE_RELEASE(swapchainRTV);

	MIRU_D3D12_SAFE_RELEASE(m_SwapchainRTVDescHeap);
	MIRU_D3D12_SAFE_RELEASE(m_Swapchain);
}

void Swapchain::Resize(uint32_t width, uint32_t height)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI.context->DeviceWaitIdle();

	m_Width = width;
	m_Height = height;

	//Resize Buffers and release old resources
	for (auto& swapchainRTV : m_SwapchainRTVs)
		MIRU_D3D12_SAFE_RELEASE(swapchainRTV);

	MIRU_D3D12_SAFE_RELEASE(m_SwapchainRTVDescHeap);
	m_SwapchainRTVs.clear();
	m_SwapchainRTV_CPU_Desc_Handles.clear();

	MIRU_ASSERT(m_Swapchain->ResizeBuffers(m_CI.swapchainCount, width, height, m_SwapchainDesc.Format, m_SwapchainDesc.Flags), "ERROR: D3D12: Failed to Resize Swapchain Images");

	//Create Swapchian RTV
	m_SwapchainRTVDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_SwapchainRTVDescHeapDesc.NumDescriptors = m_SwapchainDesc.BufferCount;
	m_SwapchainRTVDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_SwapchainRTVDescHeapDesc.NodeMask = 0;
	MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_SwapchainRTVDescHeapDesc, IID_PPV_ARGS(&m_SwapchainRTVDescHeap)), "ERROR: D3D12: Failed to Create Swapchain Image Descriptor Heap.");
	D3D12SetName(m_SwapchainRTVDescHeap, m_CI.debugName + ": RTV Descriptor Heap");

	D3D12_CPU_DESCRIPTOR_HANDLE swapchainRTV_CPUDescHandle = m_SwapchainRTVDescHeap->GetCPUDescriptorHandleForHeapStart();
	size_t rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_SwapchainRTVs.reserve(m_SwapchainRTVDescHeapDesc.NumDescriptors);
	m_SwapchainRTV_CPU_Desc_Handles.reserve(m_SwapchainRTVDescHeapDesc.NumDescriptors);
	for (UINT i = 0; i < m_SwapchainRTVDescHeapDesc.NumDescriptors; i++)
	{
		ID3D12Resource* swapchainRTV;
		MIRU_ASSERT(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchainRTV)), "ERROR: D3D12: Failed to get Swapchain Image.");
		m_Device->CreateRenderTargetView(swapchainRTV, nullptr, swapchainRTV_CPUDescHandle);
		D3D12SetName(swapchainRTV, m_CI.debugName + ": RTV " + std::to_string(i));

		m_SwapchainRTVs.push_back(swapchainRTV);
		m_SwapchainRTV_CPU_Desc_Handles.push_back(swapchainRTV_CPUDescHandle);
		swapchainRTV_CPUDescHandle.ptr += rtvDescriptorSize;
	}

	FillSwapchainImageAndViews((void**)m_SwapchainRTVs.data(), (void**)m_SwapchainRTV_CPU_Desc_Handles.data(), m_Width, m_Height, static_cast<uint32_t>(m_Format));
	m_Resized = true;
}

void Swapchain::AcquireNextImage(const base::SemaphoreRef& acquire, uint32_t& imageIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	imageIndex = static_cast<uint32_t>(m_Swapchain->GetCurrentBackBufferIndex());
}

void Swapchain::Present(const base::CommandPoolRef& cmdPool, const base::SemaphoreRef& submit, uint32_t& imageIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_CI.vSync)
	{
		MIRU_ASSERT(m_Swapchain->Present(1, 0), "ERROR: D3D12: Failed to present the Image from Swapchain.");
	}
	else
	{
		MIRU_ASSERT(m_Swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING), "ERROR: D3D12: Failed to present the Image from Swapchain.");
	}
}
#endif