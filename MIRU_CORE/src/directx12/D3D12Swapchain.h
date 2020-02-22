#pragma once
#include "crossplatform/Swapchain.h"

namespace miru
{
namespace d3d12
{
	class Swapchain final : public crossplatform::Swapchain
	{
		//Methods
	public:
		Swapchain(CreateInfo* pCreateInfo);
		~Swapchain();

	private:
		void Resize(uint32_t width, uint32_t height);

		//Members
	public:
		IDXGIFactory4* m_Factory;
		ID3D12Device* m_Device;

		IDXGISwapChain4* m_Swapchain;
		DXGI_SWAP_CHAIN_DESC1 m_SwapchainDesc;
	
		std::vector<ID3D12Resource*> m_SwapchainRTVs;
		ID3D12DescriptorHeap* m_SwapchainRTVDescHeap;
		D3D12_DESCRIPTOR_HEAP_DESC m_SwapchainRTVDescHeapDesc;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_SwapchainRTV_CPU_Desc_Handles;

		UINT m_Width, m_Height;
		DXGI_FORMAT m_Format;
	};
}
}