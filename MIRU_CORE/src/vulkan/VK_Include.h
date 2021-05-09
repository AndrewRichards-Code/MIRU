#pragma once
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