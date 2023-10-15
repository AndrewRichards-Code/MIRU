#include "VKSwapchain.h"
#include "VKContext.h"
#include "VKSync.h"
#include "VKCommandPoolBuffer.h"

using namespace miru;
using namespace vulkan;

Swapchain::Swapchain(CreateInfo* pCreateInfo)
	:m_Instance(ref_cast<Context>(pCreateInfo->context)->m_Instance),
	m_Device(ref_cast<Context>(pCreateInfo->context)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	VkPhysicalDevice physicalDevice = ref_cast<Context>(pCreateInfo->context)->m_PhysicalDevices.m_PDIs[0].m_PhysicalDevice;
	uint32_t queueFamilyIndex = 0;

	//Surface
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR m_SurfaceCI;
	m_SurfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	m_SurfaceCI.pNext = nullptr;
	m_SurfaceCI.flags = 0;
	m_SurfaceCI.hwnd = static_cast<HWND>(m_CI.pWindow);
	m_SurfaceCI.hinstance = GetModuleHandle(nullptr);

	MIRU_FATAL(vkCreateWin32SurfaceKHR(m_Instance, &m_SurfaceCI, nullptr, &m_Surface), "ERROR: VULKAN: Failed to create Win32Surface.");
	VKSetName<VkSurfaceKHR>(m_Device, m_Surface, m_CI.debugName + ": Surface");

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR m_SurfaceCI;
	m_SurfaceCI.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	m_SurfaceCI.pNext = nullptr;
	m_SurfaceCI.flags = 0;
	m_SurfaceCI.window = reinterpret_cast<ANativeWindow*>(m_CI.pWindow);

	MIRU_FATAL(vkCreateAndroidSurfaceKHR(m_Instance, &m_SurfaceCI, nullptr, &m_Surface), "ERROR: VULKAN: Failed to create AndroidSurface.");
	VKSetName<VkSurfaceKHR>(m_Device, m_Surface, m_CI.debugName + ": Surface");

#else
	MIRU_FATAL(true, "ERROR: VULKAN: Unknown Platform.");
#endif

	VkBool32 surfaceSupport;
	MIRU_FATAL(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, m_Surface, &surfaceSupport), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceSupport.");
	if (!surfaceSupport)
	{
		MIRU_FATAL(true, "ERROR: VULKAN: Can not use the created surface.");
	}

	MIRU_FATAL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &m_SurfaceCapability), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceCapabilities.");

	uint32_t surfaceFormatsCount = 0;
	MIRU_FATAL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatsCount, nullptr), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceFormats.");
	m_SurfaceFormats.resize(surfaceFormatsCount);
	MIRU_FATAL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatsCount, m_SurfaceFormats.data()), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceFormats.");

	uint32_t presentModesCount = 0;
	MIRU_FATAL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModesCount, nullptr), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfacePresentModes.");
	m_PresentModes.resize(presentModesCount);
	MIRU_FATAL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModesCount, m_PresentModes.data()), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfacePresentModes.");

	if (m_SurfaceFormats.empty() || m_PresentModes.empty())
	{
		MIRU_FATAL(true, "ERROR: VULKAN: Could not find suitable surface formats and/or present modes for the created surface");
	}

	VkSurfaceFormatKHR surfaceFormat;
	switch (m_CI.bpcColourSpace)
	{
	default:
	case base::Swapchain::BPC_ColourSpace::B8G8R8A8_UNORM_SRGB_NONLINEAR:
		surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR }; break;
	case base::Swapchain::BPC_ColourSpace::A2B10G10R10_UNORM_PACK32_SRGB_NONLINEAR:
		surfaceFormat = { VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLORSPACE_SRGB_NONLINEAR_KHR }; break;
	case base::Swapchain::BPC_ColourSpace::A2B10G10R10_UNORM_PACK32_HDR10_ST2084:
		surfaceFormat = { VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLOR_SPACE_HDR10_ST2084_EXT }; break;
	case base::Swapchain::BPC_ColourSpace::R16G16B16A16_SFLOAT_EXTENDED_SRGB_LINEAR:
		surfaceFormat = { VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT }; break;
	}
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	m_ChosenSurfaceFormat = m_SurfaceFormats[0]; //Default
	for (auto& _surfaceFormat : m_SurfaceFormats)
	{
		if (_surfaceFormat.format == surfaceFormat.format && _surfaceFormat.colorSpace == surfaceFormat.colorSpace)
		{
			m_ChosenSurfaceFormat = _surfaceFormat; break;
		}
		else
			continue;
	}
	m_ChosenPresentMode = m_PresentModes[0]; //Default
	for (auto& _presentMode : m_PresentModes)
	{
		if (_presentMode == presentMode)
		{
			m_ChosenPresentMode = _presentMode; break;
		}
		else
			continue;
	}

	//Swapchain
	if (!(m_SurfaceCapability.minImageCount > 0))
	{
		MIRU_FATAL(true, "ERROR: VULKAN: Swapchain minimum image count is not greater than 0.");
	}

	if (!(m_SurfaceCapability.minImageCount < m_CI.swapchainCount) || !(m_CI.swapchainCount < m_SurfaceCapability.maxImageCount))
		m_CI.swapchainCount = m_SurfaceCapability.minImageCount;

	m_Extent = m_SurfaceCapability.currentExtent;
	m_Format = m_ChosenSurfaceFormat.format;

	m_SwapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	m_SwapchainCI.pNext = nullptr;
	m_SwapchainCI.flags = 0;
	m_SwapchainCI.surface = m_Surface;
	m_SwapchainCI.minImageCount = m_CI.swapchainCount;
	m_SwapchainCI.imageFormat = m_ChosenSurfaceFormat.format;
	m_SwapchainCI.imageColorSpace = m_ChosenSurfaceFormat.colorSpace;
	m_SwapchainCI.imageExtent = m_SurfaceCapability.currentExtent;
	m_SwapchainCI.imageArrayLayers = 1;
	m_SwapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	m_SwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	m_SwapchainCI.queueFamilyIndexCount = 0;
	m_SwapchainCI.pQueueFamilyIndices = nullptr;
	m_SwapchainCI.preTransform = m_SurfaceCapability.currentTransform;
	m_SwapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	m_SwapchainCI.presentMode = m_CI.vSync ? m_ChosenPresentMode : VK_PRESENT_MODE_IMMEDIATE_KHR; //Use VK_PRESENT_MODE_IMMEDIATE_KHR for no v-sync.
	m_SwapchainCI.clipped = VK_TRUE;
	m_SwapchainCI.oldSwapchain = VK_NULL_HANDLE;

	MIRU_FATAL(vkCreateSwapchainKHR(m_Device, &m_SwapchainCI, nullptr, &m_Swapchain), "ERROR: VULKAN: Failed to create Swapchain");
	VKSetName<VkSwapchainKHR>(m_Device, m_Swapchain, m_CI.debugName);

	uint32_t swapchainImagesCount = 0;
	MIRU_FATAL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, 0), "ERROR: VULKAN: Failed to get Swapchain Images");
	m_SwapchainImages.resize(swapchainImagesCount);
	MIRU_FATAL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, m_SwapchainImages.data()), "ERROR: VULKAN: Failed to get Swapchain Images");

	for (size_t i = 0; i < m_SwapchainImages.size(); i++)
	{
		VKSetName<VkImage>(m_Device, m_SwapchainImages[i], m_CI.debugName + ": Image" + std::to_string(i));
		
		VkImageViewCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.image = m_SwapchainImages[i];
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = m_ChosenSurfaceFormat.format;
		ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkImageView imageView;
		MIRU_FATAL(vkCreateImageView(m_Device, &ci, 0, &imageView), "ERROR: VULKAN: Failed to create Swapchain ImageViews");
		VKSetName<VkImageView>(m_Device, imageView, m_CI.debugName + ": ImageView " + std::to_string(i));

		m_SwapchainImageViews.push_back(imageView);
	}

	std::vector<VkImage*> images;
	for (auto& swapchainImage : m_SwapchainImages)
	{
		images.push_back(&swapchainImage);
	}

	FillSwapchainImageAndViews((void**)images.data(), (void*)m_SwapchainImageViews.data(), m_Extent.width, m_Extent.height, m_Format);
}

Swapchain::~Swapchain()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto& imageView : m_SwapchainImageViews)
		vkDestroyImageView(m_Device, imageView, nullptr);
	m_SwapchainImageViews.clear();

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}

void Swapchain::Resize(uint32_t width, uint32_t height)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI.context->DeviceWaitIdle();

	//Destroy old swapchain
	for (auto& imageView : m_SwapchainImageViews)
		vkDestroyImageView(m_Device, imageView, nullptr);
	m_SwapchainImageViews.clear();

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

	//Create new swapchain
	m_SwapchainCI.imageExtent = { width, height };
	m_Extent = m_SwapchainCI.imageExtent;

	MIRU_FATAL(vkCreateSwapchainKHR(m_Device, &m_SwapchainCI, nullptr, &m_Swapchain), "ERROR: VULKAN: Failed to create Swapchain");
	VKSetName<VkSurfaceKHR>(m_Device, m_Surface, m_CI.debugName);

	uint32_t swapchainImagesCount = 0;
	MIRU_FATAL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, 0), "ERROR: VULKAN: Failed to Swapchain Images");
	m_SwapchainImages.clear();
	m_SwapchainImages.resize(swapchainImagesCount);
	MIRU_FATAL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, m_SwapchainImages.data()), "ERROR: VULKAN: Failed to Swapchain Images");

	for (size_t i = 0; i < m_SwapchainImages.size(); i++)
	{
		VKSetName<VkImage>(m_Device, m_SwapchainImages[i], m_CI.debugName + ": Image" + std::to_string(i));
		
		VkImageViewCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.image = m_SwapchainImages[i];
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = m_ChosenSurfaceFormat.format;
		ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkImageView imageView;
		MIRU_FATAL(vkCreateImageView(m_Device, &ci, 0, &imageView), "ERROR: VULKAN: Failed to create Swapchain ImageViews");
		VKSetName<VkImageView>(m_Device, imageView, m_CI.debugName + ": ImageView " + std::to_string(i));

		m_SwapchainImageViews.push_back(imageView);
	}

	FillSwapchainImageAndViews((void**)m_SwapchainImages.data(), (void*)m_SwapchainImageViews.data(), m_Extent.width, m_Extent.height, m_Format);
	m_Resized = true;
}

void Swapchain::AcquireNextImage(const base::SemaphoreRef& acquire, uint32_t& imageIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, ref_cast<Semaphore>(acquire)->m_Semaphore, VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		m_Resized = true;
	}
	else
	{
		MIRU_FATAL(result, "ERROR: VULKAN: Failed to acquire next Image from Swapchain.");
	}
}

void Swapchain::Present(const base::CommandPoolRef& cmdPool, const base::SemaphoreRef& submit, uint32_t& imageIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	const ContextRef& context = ref_cast<Context>(m_CI.context);
	const CommandPoolRef& pool = ref_cast<CommandPool>(cmdPool);

	VkQueue vkQueue = context->m_Queues[pool->GetQueueFamilyIndex(pool->GetCreateInfo().queueType)][0];

	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &ref_cast<Semaphore>(submit)->m_Semaphore;
	pi.swapchainCount = 1;
	pi.pSwapchains = &m_Swapchain;
	pi.pImageIndices = &imageIndex;
	pi.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(vkQueue, &pi);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		m_Resized = true;
	}
	else
	{
		MIRU_FATAL(result, "ERROR: VULKAN: Failed to present the Image from Swapchain.");
	}
}