#pragma once
#if defined(MIRU_VULKAN)

namespace miru
{
namespace vulkan
{
#if defined(MIRU_ALLOW_API_SETNAME_FN_COMPILE) && defined(_DEBUG)
	template<class T>
	static inline void VKSetName(VkDevice& device, const T& objectHandle, const std::string& name)
	{
		if (!crossplatform::GraphicsAPI::IsSetNameAllowed())
			return;
	
		VkObjectType objectType = VK_OBJECT_TYPE_UNKNOWN;
		if (typeid(T) == typeid(VkInstance))
			objectType = VK_OBJECT_TYPE_INSTANCE;
		else if (typeid(T) == typeid(VkPhysicalDevice))
			objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
		else if (typeid(T) == typeid(VkDevice))
			objectType = VK_OBJECT_TYPE_DEVICE;
		else if (typeid(T) == typeid(VkQueue))
			objectType = VK_OBJECT_TYPE_QUEUE;
		else if (typeid(T) == typeid(VkSemaphore))
			objectType = VK_OBJECT_TYPE_SEMAPHORE;
		else if (typeid(T) == typeid(VkCommandBuffer))
			objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
		else if (typeid(T) == typeid(VkFence))
			objectType = VK_OBJECT_TYPE_FENCE;
		else if (typeid(T) == typeid(VkDeviceMemory))
			objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
		else if (typeid(T) == typeid(VkBuffer))
			objectType = VK_OBJECT_TYPE_BUFFER;
		else if (typeid(T) == typeid(VkImage))
			objectType = VK_OBJECT_TYPE_IMAGE;
		else if (typeid(T) == typeid(VkEvent))
			objectType = VK_OBJECT_TYPE_EVENT;
		else if (typeid(T) == typeid(VkQueryPool))
			objectType = VK_OBJECT_TYPE_QUERY_POOL;
		else if (typeid(T) == typeid(VkBufferView))
			objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
		else if (typeid(T) == typeid(VkImageView))
			objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
		else if (typeid(T) == typeid(VkShaderModule))
			objectType = VK_OBJECT_TYPE_SHADER_MODULE;
		else if (typeid(T) == typeid(VkPipelineCache))
			objectType = VK_OBJECT_TYPE_PIPELINE_CACHE;
		else if (typeid(T) == typeid(VkPipelineLayout))
			objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		else if (typeid(T) == typeid(VkRenderPass))
			objectType = VK_OBJECT_TYPE_RENDER_PASS;
		else if (typeid(T) == typeid(VkPipeline))
			objectType = VK_OBJECT_TYPE_PIPELINE;
		else if (typeid(T) == typeid(VkDescriptorSetLayout))
			objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
		else if (typeid(T) == typeid(VkSampler))
			objectType = VK_OBJECT_TYPE_SAMPLER;
		else if (typeid(T) == typeid(VkDescriptorPool))
			objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
		else if (typeid(T) == typeid(VkDescriptorSet))
			objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		else if (typeid(T) == typeid(VkFramebuffer))
			objectType = VK_OBJECT_TYPE_FRAMEBUFFER;
		else if (typeid(T) == typeid(VkCommandPool))
			objectType = VK_OBJECT_TYPE_COMMAND_POOL;
		else if (typeid(T) == typeid(VkSurfaceKHR))
			objectType = VK_OBJECT_TYPE_SURFACE_KHR;
		else if (typeid(T) == typeid(VkSwapchainKHR))
			objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
		else if (typeid(T) == typeid(VkAccelerationStructureKHR))
			objectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
		else
			objectType = VK_OBJECT_TYPE_UNKNOWN;
	
		VkDebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = reinterpret_cast<uint64_t>(objectHandle);
		nameInfo.pObjectName = name.c_str();
	
		if (vkSetDebugUtilsObjectNameEXT)
			vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
	}
#else
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	template<class T>
	static inline void VKSetName(VkDevice& device, const T& objectHandle, const std::string& name) { return; }
#endif
}
}
#endif
