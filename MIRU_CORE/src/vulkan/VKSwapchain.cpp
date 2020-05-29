#include "miru_core_common.h"
#include "VKSwapchain.h"
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

Swapchain::Swapchain(CreateInfo* pCreateInfo)
	:m_Instance(ref_cast<Context>(pCreateInfo->pContext)->m_Instance),
	m_Device(ref_cast<Context>(pCreateInfo->pContext)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	VkPhysicalDevice physicalDevice = ref_cast<Context>(pCreateInfo->pContext)->m_PhysicalDevices.m_PhysicalDevices[0];
	uint32_t queueFamilyIndex = 0;

	//Surface
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR m_SurfaceCI;
	m_SurfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	m_SurfaceCI.pNext = nullptr;
	m_SurfaceCI.flags = 0;
	m_SurfaceCI.hwnd = static_cast<HWND>(m_CI.pWindow);
	m_SurfaceCI.hinstance = GetModuleHandle(nullptr);

	MIRU_ASSERT(vkCreateWin32SurfaceKHR(m_Instance, &m_SurfaceCI, nullptr, &m_Surface), "ERROR: VULKAN: Failed to create Win32Surface.");
	VKSetName<VkSurfaceKHR>(m_Device, (uint64_t)m_Surface, (std::string(m_CI.debugName) + ": Surface").c_str());

#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR m_SurfaceCI;
	m_SurfaceCI.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	m_SurfaceCI.pNext = nullptr;
	m_SurfaceCI.flags = 0;
	m_SurfaceCI.window = reinterpret_cast<ANativeWindow*>(m_CI.pWindow);

	MIRU_ASSERT(vkCreateAndroidSurfaceKHR(m_Instance, &m_SurfaceCI, nullptr, &m_Surface), "ERROR: VULKAN: Failed to create AndroidSurface.");
	VKSetName<VkSurfaceKHR>(m_Device, (uint64_t)m_Surface, (std::string(m_CI.debugName) + ": Surface").c_str());

#else
	MIRU_ASSERT(true, "ERROR: VULKAN: Unknown Platform.");
#endif

	VkBool32 surfaceSupport;
	MIRU_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, m_Surface, &surfaceSupport), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceSupport.");
	if (!surfaceSupport)
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: Can not use the created surface.");
	}

	MIRU_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &m_SurfaceCapability), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceCapabilities.");

	uint32_t surfaceFormatsCount = 0;
	MIRU_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatsCount, 0), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceFormats.");
	m_SurfaceFormats.resize(surfaceFormatsCount);
	MIRU_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatsCount, m_SurfaceFormats.data()), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfaceFormats.");

	uint32_t presentModesCount = 0;
	MIRU_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModesCount, 0), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfacePresentModes.");
	m_PresentModes.resize(presentModesCount);
	MIRU_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModesCount, m_PresentModes.data()), "ERROR: VULKAN: Failed to get PhysicalDeviceSurfacePresentModes.");

	if (m_SurfaceFormats.empty() || m_PresentModes.empty())
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: Could not find suitable surface formats and/or present modes for the created surface");
	}

	VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
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
		MIRU_ASSERT(true, "ERROR: VULKAN: Swapchain minimum image count is not greater than 0.");
	}

	if (!(m_SurfaceCapability.minImageCount < m_CI.swapchainCount) && !(m_CI.swapchainCount < m_SurfaceCapability.maxImageCount))
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

	MIRU_ASSERT(vkCreateSwapchainKHR(m_Device, &m_SwapchainCI, nullptr, &m_Swapchain), "ERROR: VULKAN: Failed to create Swapchain");
	VKSetName<VkSwapchainKHR>(m_Device, (uint64_t)m_Swapchain, m_CI.debugName);

	uint32_t swapchainImagesCount = 0;
	MIRU_ASSERT(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, 0), "ERROR: VULKAN: Failed to get Swapchain Images");
	m_SwapchainImages.resize(swapchainImagesCount);
	MIRU_ASSERT(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, m_SwapchainImages.data()), "ERROR: VULKAN: Failed to get Swapchain Images");

	for (size_t i = 0; i < m_SwapchainImages.size(); i++)
	{
		VKSetName<VkImage>(m_Device, (uint64_t)m_SwapchainImages[i], (std::string(m_CI.debugName) + ": Image" + std::to_string(i)).c_str());
		
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
		MIRU_ASSERT(vkCreateImageView(m_Device, &ci, 0, &imageView), "ERROR: VULKAN: Failed to create Swapchain ImageViews");
		VKSetName<VkImageView>(m_Device, (uint64_t)imageView, (std::string(m_CI.debugName) + ": ImageView " + std::to_string(i)).c_str());

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

	m_CI.pContext->DeviceWaitIdle();

	//Destroy old swapchain
	for (auto& imageView : m_SwapchainImageViews)
		vkDestroyImageView(m_Device, imageView, 0);
	m_SwapchainImageViews.clear();

	vkDestroySwapchainKHR(m_Device, m_Swapchain, 0);

	//Create new swapchain
	m_SwapchainCI.imageExtent = { width, height };

	MIRU_ASSERT(vkCreateSwapchainKHR(m_Device, &m_SwapchainCI, nullptr, &m_Swapchain), "ERROR: VULKAN: Failed to create Swapchain");
	VKSetName<VkSurfaceKHR>(m_Device, (uint64_t)m_Surface, m_CI.debugName);

	uint32_t swapchainImagesCount = 0;
	MIRU_ASSERT(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, 0), "ERROR: VULKAN: Failed to Swapchain Images");
	m_SwapchainImages.clear();
	m_SwapchainImages.resize(swapchainImagesCount);
	MIRU_ASSERT(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImagesCount, m_SwapchainImages.data()), "ERROR: VULKAN: Failed to Swapchain Images");

	for (size_t i = 0; i < m_SwapchainImages.size(); i++)
	{
		VKSetName<VkImage>(m_Device, (uint64_t)m_SwapchainImages[i], (std::string(m_CI.debugName) + ": Image" + std::to_string(i)).c_str());
		
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
		MIRU_ASSERT(vkCreateImageView(m_Device, &ci, 0, &imageView), "ERROR: VULKAN: Failed to create Swapchain ImageViews");
		VKSetName<VkImageView>(m_Device, (uint64_t)imageView, (std::string(m_CI.debugName) + ": ImageView " + std::to_string(i)).c_str());

		m_SwapchainImageViews.push_back(imageView);
	}

	std::vector<VkImage*> images;
	for (auto& swapchainImage : m_SwapchainImages)
	{
		images.push_back(&swapchainImage);
	}

	FillSwapchainImageAndViews((void**)images.data(), (void*)m_SwapchainImageViews.data(), m_Extent.width, m_Extent.height, m_Format);
	m_Resized = true;
}