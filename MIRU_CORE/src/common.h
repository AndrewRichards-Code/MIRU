#pragma once
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum' warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable' warning C26495

//CSTDLIB
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <assert.h>

//WINDOWING SYSTEM
#if defined(_WIN64)
#include <Windows.h>
#else
#endif

//GRAPHICS API
#if defined(_WIN64)
#define MIRU_D3D12
#define MIRU_VULKAN
#elif defined(__APPLE__)
#define MIRU_METAL //Use Metal?
#efif defined(__linux__)
#define MIRU_VULKAN
#endif

//D3D12
#if defined(MIRU_D3D12)
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define SAFE_RELEASE(x) if(x) { x->Release(); x = nullptr; }

#endif

//Vulkan
#if defined(MIRU_VULKAN)
#if defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkan/vulkan.h"
#pragma comment(lib, "vulkan-1.lib")
/*char* buffer = nullptr;
size_t size = 0;
_dupenv_s(&buffer, &size, "VULKAN_SDK");
std::string vulkanSDKDir = (buffer != nullptr) ? std::string(buffer) : std::string("");*/
#endif

#include "crossplatform/GraphicsAPI.h"

//MIRU Helpers
namespace miru
{
	template<class T>
	using Scope = std::unique_ptr<T>;

	template<class T>
	using Ref = std::shared_ptr<T>;
}

//MIRU Enum Class Bitwise Operators Templates
#define MIRU_ENUM_CLASS_BITWISE_OPERATORS
#if defined(MIRU_ENUM_CLASS_BITWISE_OPERATORS)
template<typename _Ty>
_Ty operator|(_Ty lhs, _Ty rhs)
{
return static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) | static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
_Ty operator&(_Ty lhs, _Ty rhs)
{
return static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs)& static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
_Ty operator^(_Ty lhs, _Ty rhs)
{
return static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) ^ static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
_Ty operator~(_Ty rhs)
{
return static_cast<_Ty>(~static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
_Ty& operator|=(_Ty& lhs, _Ty rhs)
{
lhs = static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) | static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
return lhs; 
}
template<typename _Ty>
_Ty& operator&=(_Ty& lhs, _Ty rhs)
{
lhs = static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs)& static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
return lhs; 
}
template<typename _Ty>
_Ty& operator^=(_Ty& lhs, _Ty rhs)
{
lhs = static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) ^ static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
return lhs; 
}
#endif

//MIRU SetName
#if defined(MIRU_ALLOW_API_SETNAME_FN_COMPILE)
namespace miru
{
	static inline void D3D12SetName(void* object, const char* name)
	{
		if (!GraphicsAPI::GetUseSetName())
			return;

		reinterpret_cast<ID3D12Object*>(object)->SetName(reinterpret_cast<LPCWSTR>(name));
	}

	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	template<class T>
	static inline void VKSetName(VkDevice& device, uint64_t objectHandle, const char* name)
	{
		if (!GraphicsAPI::GetUseSetName())
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
		else
			objectType = VK_OBJECT_TYPE_UNKNOWN;

		VkDebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = objectHandle;
		nameInfo.pObjectName = name;

		vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
	}
}
#else
namespace miru
{
	static inline void D3D12SetName(void* object, const char* name) { return; }

	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	template<class T>
	static inline void VKSetName(VkDevice& device, uint64_t objectHandle, const char* name) { return; }
}
#endif

//MIRU Debugbreak, Assert and Warn
#if defined(_MSC_VER)
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK raise(SIGTRAP);
#endif

//Triggered if x != 0
#define MIRU_ASSERT(x, y) if(x != 0) { printf("MIRU_ASSERT: %s(%d): ERROR_CODE: %d(0x%x) - %s\n", __FILE__, __LINE__, static_cast<int>(x), static_cast<int>(x), y); DEBUG_BREAK; }
#define MIRU_WARN(x, y) if(x != 0) { printf("MIRU_WARN: %s(%d): ERROR_CODE: %d(0x%x) - %s\n", __FILE__, __LINE__, static_cast<int>(x), static_cast<int>(x), y); }