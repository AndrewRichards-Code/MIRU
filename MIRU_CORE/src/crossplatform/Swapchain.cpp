#include "common.h"
#include "directx12/D3D12Swapchain.h"
#include "vulkan/VKSwapchain.h"

using namespace miru;
using namespace crossplatform;

Ref<Swapchain> Swapchain::Create(Swapchain::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return std::make_shared<d3d12::Swapchain>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return std::make_shared<vulkan::Swapchain>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

#include "directx12/D3D12Image.h"
#include "vulkan/VKImage.h"

void Swapchain::FillSwapchainImages(void** pImages)
{
	m_SwapchianImages.resize(m_CI.swapchainCount);

	Image::CreateInfo swapchainImageCI;
	swapchainImageCI.debugName = "";
	swapchainImageCI.device = m_CI.pContext->GetDevice();
	swapchainImageCI.type = Image::Type::TYPE_2D;
	swapchainImageCI.format = Image::Format::B8G8R8A8_UNORM;
	swapchainImageCI.width = m_CI.width;
	swapchainImageCI.height = m_CI.height;
	swapchainImageCI.depth = 1;
	swapchainImageCI.mipLevels = 1;
	swapchainImageCI.arrayLayers = 1;
	swapchainImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	swapchainImageCI.usage = Image::UsageBit::TRANSFER_DST_BIT;
	swapchainImageCI.layout = Image::Layout::UNKNOWN;
	swapchainImageCI.size = 0;
	swapchainImageCI.data = nullptr;
	swapchainImageCI.pMemoryBlock = nullptr;
	
	size_t i = 0;
	for (auto& swapchainImage : m_SwapchianImages)
	{
		if (GraphicsAPI::GetAPI() == GraphicsAPI::API::D3D12)
		{
			swapchainImage = std::make_shared<d3d12::Image>();
			swapchainImage->m_CI = swapchainImageCI;
			swapchainImage->m_SwapchainImage = true;
			std::dynamic_pointer_cast<d3d12::Image>(swapchainImage)->m_Image = reinterpret_cast<ID3D12Resource*>(pImages[i]);
			std::dynamic_pointer_cast<d3d12::Image>(swapchainImage)->m_CurrentResourceState = D3D12_RESOURCE_STATE_COMMON;
		}
		else
		{
			swapchainImage = std::make_shared<vulkan::Image>();
			swapchainImage->m_CI = swapchainImageCI;
			swapchainImage->m_SwapchainImage = true;
			std::dynamic_pointer_cast<vulkan::Image>(swapchainImage)->m_Image = *reinterpret_cast<VkImage*>(pImages[i]);
			std::dynamic_pointer_cast<vulkan::Image>(swapchainImage)->m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		i++;
	}
}
