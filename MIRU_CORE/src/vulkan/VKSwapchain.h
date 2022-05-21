#pragma once
#if defined(MIRU_VULKAN)
#include "base/Swapchain.h"

namespace miru
{
namespace vulkan
{
	class Swapchain final : public base::Swapchain
	{
		//Methods
	public:
		Swapchain(CreateInfo* pCreateInfo);
		~Swapchain();

		void Resize(uint32_t width, uint32_t height) override;
		void AcquireNextImage(const Ref<base::Semaphore>& acquire, uint32_t& imageIndex) override;
		void Present(const Ref<base::CommandPool>& cmdPool, const Ref<base::Semaphore>& submit, uint32_t& imageIndex) override;

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
#endif