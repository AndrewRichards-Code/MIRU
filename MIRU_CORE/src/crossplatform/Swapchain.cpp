#include "miru_core_common.h"
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
#include "crossplatform/Context.h"

void Swapchain::FillSwapchainImageAndViews(void** pImages, void* pImageViews, uint32_t width, uint32_t height)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_SwapchainImages.resize(m_CI.swapchainCount);
	m_SwapchainImageViews.resize(m_CI.swapchainCount);

	Image::CreateInfo swapchainImageCI;
	swapchainImageCI.debugName = "";
	swapchainImageCI.device = m_CI.pContext->GetDevice();
	swapchainImageCI.type = Image::Type::TYPE_2D;
	swapchainImageCI.format = Image::Format::B8G8R8A8_UNORM;
	swapchainImageCI.width = width;
	swapchainImageCI.height = height;
	swapchainImageCI.depth = 1;
	swapchainImageCI.mipLevels = 1;
	swapchainImageCI.arrayLayers = 1;
	swapchainImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	swapchainImageCI.usage = Image::UsageBit::TRANSFER_DST_BIT;
	swapchainImageCI.layout = Image::Layout::UNKNOWN;
	swapchainImageCI.size = 0;
	swapchainImageCI.data = nullptr;
	swapchainImageCI.pMemoryBlock = nullptr;

	ImageView::CreateInfo swapchainImageViewCI;
	swapchainImageViewCI.debugName = "";
	swapchainImageViewCI.device = m_CI.pContext->GetDevice();
	swapchainImageViewCI.pImage = nullptr;
	swapchainImageViewCI.subresourceRange = { Image::AspectBit::COLOR_BIT, 0, 1, 0, 1 };

	size_t i = 0;
	for (auto& swapchainImage : m_SwapchainImages)
	{
		if (GraphicsAPI::GetAPI() == GraphicsAPI::API::D3D12)
		{
			swapchainImage = std::make_shared<d3d12::Image>();
			swapchainImage->m_CI = swapchainImageCI;
			swapchainImage->m_SwapchainImage = true;
			ref_cast<d3d12::Image>(swapchainImage)->m_Image = reinterpret_cast<ID3D12Resource*>(pImages[i]);
			ref_cast<d3d12::Image>(swapchainImage)->m_CurrentResourceState = D3D12_RESOURCE_STATE_COMMON;
			
			swapchainImageViewCI.pImage = swapchainImage;
			m_SwapchainImageViews[i] = std::make_shared<d3d12::ImageView>();
			m_SwapchainImageViews[i]->m_CI = swapchainImageViewCI;
			m_SwapchainImageViews[i]->m_SwapchainImageView = true;
			ref_cast<d3d12::ImageView>(m_SwapchainImageViews[i])->m_RTVDescHandle = reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(pImageViews)[i];
		}
		else
		{
			swapchainImage = std::make_shared<vulkan::Image>();
			swapchainImage->m_CI = swapchainImageCI;
			swapchainImage->m_SwapchainImage = true;
			ref_cast<vulkan::Image>(swapchainImage)->m_Image = *reinterpret_cast<VkImage*>(pImages[i]);
			ref_cast<vulkan::Image>(swapchainImage)->m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			
			swapchainImageViewCI.pImage = swapchainImage;
			m_SwapchainImageViews[i] = std::make_shared<vulkan::ImageView>();
			m_SwapchainImageViews[i]->m_CI = swapchainImageViewCI;
			m_SwapchainImageViews[i]->m_SwapchainImageView = true;
			ref_cast<vulkan::ImageView>(m_SwapchainImageViews[i])->m_ImageView = reinterpret_cast<VkImageView*>(pImageViews)[i];
		}
		i++;
	}
}
