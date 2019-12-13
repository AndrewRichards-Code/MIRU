#pragma once
#include "crossplatform/Swapchain.h"

namespace miru
{
namespace vulkan
{
	class Swapchain final : public crossplatform::Swapchain
	{
		//Methods
	public:
		Swapchain(CreateInfo* pCreateInfo);
		~Swapchain();

		void Resize(uint32_t width, uint32_t height) override;

		//Members
	public:
		VkInstance& m_Instance;
		VkDevice& m_Device;

		//Surface
		VkSurfaceKHR m_Surface;
		VkSurfaceCapabilitiesKHR m_SurfaceCapability;
		std::vector<VkSurfaceFormatKHR> m_SurfaceFormats;
		std::vector<VkPresentModeKHR> m_PresentModes;
		VkSurfaceFormatKHR m_ChosenSurfaceFormat;
		VkPresentModeKHR m_ChosenPresentMode;

		//Swapchain
		VkSwapchainKHR m_Swapchain;
		VkSwapchainCreateInfoKHR m_SwapchainCI;
		
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkImage> m_SwapchainImages;

		VkExtent2D m_Extent;
		VkFormat m_Format;
	};
}
}