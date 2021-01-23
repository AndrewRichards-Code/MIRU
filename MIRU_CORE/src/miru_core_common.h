#pragma once
#if defined(_MSC_VER)
#pragma warning(disable : 26812) //Disables 'Prefered scoped enum' warning C26812
#pragma warning(disable : 26495) //Disables 'Unitialised variable' warning C26495
#pragma warning(disable : 26451) //Disables  'Arithmetic overflow' warning C26451
#endif

//CSTDLIB
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <assert.h>
#include <cmath>
#include <filesystem>

//PLATORM SYSTEM
#if defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#elif defined (__ANDROID__) && (__ANDROID_API__ >= 24)
#include <dlfcn.h>
#include <jni.h>
#include <errno.h>
#include <sys/resource.h>
#include <android/log.h>
#include <android/sensor.h>
#include <android/window.h>
#include <android_native_app_glue.h>
#endif

//GRAPHICS API
#if !defined(MIRU_D3D12) && !defined(MIRU_VULKAN)

#if defined(_WIN64)
#define MIRU_D3D12
#if !defined(MIRU_WIN64_UWP)
#define MIRU_VULKAN
#endif
#elif defined(__APPLE__)
#define MIRU_METAL //Use Metal?
#elif defined(__linux__) && !defined(__ANDROID__)
#define MIRU_VULKAN
#elif defined (__ANDROID__) && (__ANDROID_API__ >= 24)
#define MIRU_VULKAN
#endif

#endif

//D3D12
#if defined(MIRU_D3D12)
//Header and Library
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//D3D12MemoryAllocator
#include "D3D12MemoryAllocator/src/D3D12MemAlloc.h"

//DXC Header and Library
#include <d3d12shader.h>
#include <dxcapi.h>
//#pragma comment(lib, "dxc/lib/x64/dxcompiler.lib")

#define MIRU_D3D12_SAFE_RELEASE(x) if(x) { x->Release(); x = nullptr; }

#endif

//Vulkan
#if defined(MIRU_VULKAN)
//Platform usage
#if defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR

//Header and Library
#include "vulkan/vulkan.h"
#pragma comment(lib, "vulkan-1.lib")

//VulkanMemoryAllocator
#include "VulkanMemoryAllocator/src/vk_mem_alloc.h"

//Spirv-cross Header and Library
#include "spirv_cross/Include/spirv_cross.hpp"
#if defined(_DEBUG)
#pragma comment(lib, "spirv_cross/lib/x64/spirv-cross-cored.lib")
#else
#pragma comment(lib, "spirv_cross/lib/x64/spirv-cross-core.lib")
#endif

#elif defined(__ANDROID__)
#if !defined( VK_USE_PLATFORM_ANDROID_KHR)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

//Header and Library
#include "vulkan_wrapper.h"

//Spirv-cross Header and Library
#include "spirv_cross/Include/spirv_cross.hpp"

#endif
#endif

#include "crossplatform/GraphicsAPI.h"

//MIRU Helpers
namespace miru
{
	template<class T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<class T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<class _Ty1, class _Ty2>
	inline constexpr Ref<_Ty1> ref_cast(const Ref<_Ty2>& x) noexcept { return std::dynamic_pointer_cast<_Ty1>(x); }

	auto shrink_uint32_t_to_uint8_t = [](uint32_t a) -> uint8_t
	{
		uint8_t b = 0;
		for (int i = 0; i < 8; i++)
			b += ((0x0F & (a >> i * 4)) / 0xF) << i;
		return b;
	};
}

//MIRU Enum Class Bitwise Operators Templates
#define MIRU_ENUM_CLASS_BITWISE_OPERATORS
#if defined(MIRU_ENUM_CLASS_BITWISE_OPERATORS)
template<typename _Ty>
constexpr _Ty operator|(_Ty lhs, _Ty rhs)
{
return static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) | static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
constexpr _Ty operator&(_Ty lhs, _Ty rhs)
{
return static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs)& static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
constexpr _Ty operator^(_Ty lhs, _Ty rhs)
{
return static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) ^ static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
constexpr _Ty operator~(_Ty rhs)
{
return static_cast<_Ty>(~static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
}
template<typename _Ty>
constexpr _Ty& operator|=(_Ty& lhs, _Ty rhs)
{
lhs = static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) | static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
return lhs; 
}
template<typename _Ty>
constexpr _Ty& operator&=(_Ty& lhs, _Ty rhs)
{
lhs = static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs)& static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
return lhs; 
}
template<typename _Ty>
constexpr _Ty& operator^=(_Ty& lhs, _Ty rhs)
{
lhs = static_cast<_Ty>(static_cast<typename std::underlying_type<_Ty>::type>(lhs) ^ static_cast<typename std::underlying_type<_Ty>::type>(rhs)); 
return lhs; 
}
#endif

//MIRU CPU Heap Allocation tracker
//#define MIRU_CPU_HEAP_ALLOCATION_TRACKER
#if defined(MIRU_CPU_HEAP_ALLOCATION_TRACKER)
void* operator new(size_t size);
void operator delete(void* ptr);
namespace miru
{
	class CPU_HEAP_ALLOC_TRACKER
	{
	public:
		static size_t current_allocated_bytes;
		//static std::map<void*, size_t> current_allocation_map;
	};
}
#endif

//MIRU CPU Profiler
#define MIRU_CPU_PROFILER
#if defined(MIRU_CPU_PROFILER)
namespace miru
{
	class Timer
	{
		//enum/struct
		struct ProfileDatum
		{
			uint64_t scopeNumber;
			std::string name;
			double duration;
		};

		//Methods
	public:
		Timer(const std::string& name);
		~Timer();

		void Stop();

		static void BeginSession(const std::string& filepath);
		static void EndSession();

		//Members
	private:
		std::string m_Name;
		bool m_Stopped = false;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTP, m_EndTP;

		static uint64_t m_ScopeCount;
		static std::vector<ProfileDatum> m_ProfileData;
		static std::fstream m_File;
	};

}
#if defined(_DEBUG)
#define MIRU_CPU_PROFILE_BEGIN_SESSION(filepath) miru::Timer::BeginSession(filepath);
#define MIRU_CPU_PROFILE_END_SESSION() miru::Timer::EndSession();
#define MIRU_CPU_PROFILE_SCOPE(name) miru::Timer timer##__LINE__(name)
#if(_WIN64)
#define MIRU_CPU_PROFILE_FUNCTION() MIRU_CPU_PROFILE_SCOPE(__FUNCSIG__)
#elif(__linux__)
#define MIRU_CPU_PROFILE_FUNCTION() MIRU_CPU_PROFILE_SCOPE(__PRETTY_FUNCTION__)
#endif
#else
#define MIRU_CPU_PROFILE_BEGIN_SESSION(filepath);
#define MIRU_CPU_PROFILE_END_SESSION();
#define MIRU_CPU_PROFILE_SCOPE(name)
#define MIRU_CPU_PROFILE_FUNCTION()
#endif
#endif

//MIRU SetName
#define MIRU_ALLOW_API_SETNAME_FN_COMPILE
#if defined(MIRU_ALLOW_API_SETNAME_FN_COMPILE) && defined(_DEBUG)
namespace miru
{
	#if defined(MIRU_D3D12)
	static inline void D3D12SetName(void* object, const std::string& name)
	{
		if (!crossplatform::GraphicsAPI::IsSetNameAllowed())
			return;

		std::wstring w_name(name.begin(), name.end());

		reinterpret_cast<ID3D12Object*>(object)->SetName(w_name.c_str());
	}
	#endif

	#if defined(MIRU_VULKAN)
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	template<class T>
	static inline void VKSetName(VkDevice& device, uint64_t objectHandle, const std::string& name)
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
		else
			objectType = VK_OBJECT_TYPE_UNKNOWN;

		VkDebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = objectHandle;
		nameInfo.pObjectName = name.c_str();

		if(vkSetDebugUtilsObjectNameEXT)
			vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
	}
	#endif
}
#else
namespace miru
{
	#if defined(MIRU_D3D12)
	static inline void D3D12SetName(void* object, const std::string& name) { return; }
	#endif

	#if defined(MIRU_VULKAN)
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	template<class T>
	static inline void VKSetName(VkDevice& device, uint64_t objectHandle, const std::string& name) { return; }
	#endif
}
#endif

//MIRU printf
#if defined(_DEBUG)
#if !defined(__ANDROID__)
#define MIRU_PRINTF(s, ...) printf((s), __VA_ARGS__)
#else
#define MIRU_PRINTF(s, ...) __android_log_print(ANDROID_LOG_DEBUG, "MIRU_CORE", s, __VA_ARGS__)
#endif
#else
#define MIRU_PRINTF(s, ...) printf("")
#endif

//MIRU Debugbreak, Assert and Warn
#if defined(_MSC_VER)
#define DEBUG_BREAK __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP)
#endif

//Triggered if x != 0
#define MIRU_ASSERT(x, y) if(x != 0) { MIRU_PRINTF("MIRU_ASSERT: %s(%d): ERROR_CODE: %d(0x%x) - %s\n", __FILE__, __LINE__, static_cast<int>(x), static_cast<int>(x), y); DEBUG_BREAK; }
#define MIRU_WARN(x, y) if(x != 0) { MIRU_PRINTF("MIRU_WARN: %s(%d): ERROR_CODE: %d(0x%x) - %s\n", __FILE__, __LINE__, static_cast<int>(x), static_cast<int>(x), y); }
