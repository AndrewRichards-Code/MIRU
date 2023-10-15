#pragma once
//Platform usage
#if defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

//Vulkan Header
#include "vulkan/vulkan.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR) && !defined(VK_VERSION_1_3)
#error Please update Vulkan SDK to version 1.3 or later. Download from https://vulkan.lunarg.com/sdk/home.
#endif

//VulkanMemoryAllocator
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"

//SPIR-V Cross Core
#include "spirv_cross/spirv_cross.hpp"

//Internal Version
#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define MIRU_VK_API_VERSION_1_3 1
#define MIRU_VK_API_VERSION_1_2	1
#define MIRU_VK_API_VERSION_1_1	1
#define MIRU_VK_API_VERSION_1_0	1
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#define MIRU_VK_API_VERSION_1_3 0
#define MIRU_VK_API_VERSION_1_2	0
#define MIRU_VK_API_VERSION_1_1	1
#define MIRU_VK_API_VERSION_1_0	1
#endif

//Vulkan VkResult to string
#include "ARC/External/magic_enum/magic_enum.hpp"
namespace miru
{
	namespace vulkan
	{
		static std::string VkResultToString(int64_t code)
		{
			return std::string(magic_enum::enum_name(static_cast<VkResult>(code)));
		}
	}
}

//Vulkan Extension
#define MIRU_PFN_DEFINITION_LOAD(fn) inline PFN_##fn fn = ::fn
#define MIRU_PFN_DEFINITION_NULL(fn) inline PFN_##fn fn = nullptr
#define MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(fn)\
if(!fn)\
{\
	fn = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn);\
	if (!fn)\
		return false;\
}
#define MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(fn)\
if (!fn)\
{\
	fn = (PFN_##fn)vkGetDeviceProcAddr(device, #fn);\
	if (!fn)\
		return false;\
}

namespace miru
{
	namespace vulkan
	{
		//Current Extensions

		//VK_KHR_get_physical_device_properties2 - Promoted to Vulkan 1.1
#if !defined(MIRU_VK_API_VERSION_1_1)
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceFeatures2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceFormatProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceImageFormatProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceMemoryProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceQueueFamilyProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceSparseImageFormatProperties2);
#else
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceFeatures2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceFormatProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceImageFormatProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceMemoryProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceQueueFamilyProperties2);
		MIRU_PFN_DEFINITION_LOAD(vkGetPhysicalDeviceSparseImageFormatProperties2);
#endif
		inline bool LoadPFN_VK_KHR_get_physical_device_properties2(VkInstance& instance)
		{
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceFeatures2);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceFormatProperties2);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceImageFormatProperties2);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceMemoryProperties2);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceProperties2);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceQueueFamilyProperties2);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceSparseImageFormatProperties2);

			return true;
		}

		//VK_EXT_debug_utils - Requires support for Vulkan 1.0

		MIRU_PFN_DEFINITION_NULL(vkSetDebugUtilsObjectNameEXT);
		MIRU_PFN_DEFINITION_NULL(vkSetDebugUtilsObjectTagEXT);
		MIRU_PFN_DEFINITION_NULL(vkQueueBeginDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NULL(vkQueueEndDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NULL(vkQueueInsertDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NULL(vkCmdBeginDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NULL(vkCmdEndDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NULL(vkCmdInsertDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NULL(vkCreateDebugUtilsMessengerEXT);
		MIRU_PFN_DEFINITION_NULL(vkDestroyDebugUtilsMessengerEXT);
		MIRU_PFN_DEFINITION_NULL(vkSubmitDebugUtilsMessageEXT);

		inline bool LoadPFN_VK_EXT_debug_utils(VkInstance& instance)
		{
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkSetDebugUtilsObjectNameEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkSetDebugUtilsObjectTagEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkQueueBeginDebugUtilsLabelEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkQueueEndDebugUtilsLabelEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkQueueInsertDebugUtilsLabelEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkCmdBeginDebugUtilsLabelEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkCmdEndDebugUtilsLabelEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkCmdInsertDebugUtilsLabelEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkCreateDebugUtilsMessengerEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkDestroyDebugUtilsMessengerEXT);
			MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(vkSubmitDebugUtilsMessageEXT);

			return true;
		}

		//VK_KHR_ray_tracing_pipeline - Requires support for Vulkan 1.1

		MIRU_PFN_DEFINITION_NULL(vkCmdTraceRaysKHR);
		MIRU_PFN_DEFINITION_NULL(vkCreateRayTracingPipelinesKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdTraceRaysIndirectKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetRayTracingShaderGroupStackSizeKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdSetRayTracingPipelineStackSizeKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetRayTracingShaderGroupHandlesKHR);

		inline bool LoadPFN_VK_KHR_ray_tracing_pipeline(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdTraceRaysKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCreateRayTracingPipelinesKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdTraceRaysIndirectKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetRayTracingShaderGroupStackSizeKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdSetRayTracingPipelineStackSizeKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetRayTracingShaderGroupHandlesKHR);

			return true;
		}

		//VK_KHR_acceleration_structure - Requires support for Vulkan 1.1

		MIRU_PFN_DEFINITION_NULL(vkCreateAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NULL(vkDestroyAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdBuildAccelerationStructuresKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdBuildAccelerationStructuresIndirectKHR);
		MIRU_PFN_DEFINITION_NULL(vkBuildAccelerationStructuresKHR);
		MIRU_PFN_DEFINITION_NULL(vkCopyAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NULL(vkCopyAccelerationStructureToMemoryKHR);
		MIRU_PFN_DEFINITION_NULL(vkCopyMemoryToAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NULL(vkWriteAccelerationStructuresPropertiesKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdCopyAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdCopyAccelerationStructureToMemoryKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdCopyMemoryToAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetAccelerationStructureDeviceAddressKHR);
		MIRU_PFN_DEFINITION_NULL(vkCmdWriteAccelerationStructuresPropertiesKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetDeviceAccelerationStructureCompatibilityKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetAccelerationStructureBuildSizesKHR);

		inline bool LoadPFN_VK_KHR_acceleration_structure(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCreateAccelerationStructureKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkDestroyAccelerationStructureKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdBuildAccelerationStructuresKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdBuildAccelerationStructuresIndirectKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkBuildAccelerationStructuresKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCopyAccelerationStructureKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCopyAccelerationStructureToMemoryKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCopyMemoryToAccelerationStructureKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkWriteAccelerationStructuresPropertiesKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdCopyAccelerationStructureKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdCopyAccelerationStructureToMemoryKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdCopyMemoryToAccelerationStructureKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetAccelerationStructureDeviceAddressKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdWriteAccelerationStructuresPropertiesKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeviceAccelerationStructureCompatibilityKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetAccelerationStructureBuildSizesKHR);

			return true;
		}

		//VK_KHR_buffer_device_address - Promoted to Vulkan 1.2
#if !defined(MIRU_VK_API_VERSION_1_2)
		MIRU_PFN_DEFINITION_LOAD(vkGetBufferDeviceAddress);
		MIRU_PFN_DEFINITION_LOAD(vkGetBufferOpaqueCaptureAddress);
		MIRU_PFN_DEFINITION_LOAD(vkGetDeviceMemoryOpaqueCaptureAddress);
#else
		MIRU_PFN_DEFINITION_NULL(vkGetBufferDeviceAddress);
		MIRU_PFN_DEFINITION_NULL(vkGetBufferOpaqueCaptureAddress);
		MIRU_PFN_DEFINITION_NULL(vkGetDeviceMemoryOpaqueCaptureAddress);
#endif
		inline bool LoadPFN_VK_KHR_buffer_device_address(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetBufferDeviceAddress);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetBufferOpaqueCaptureAddress);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeviceMemoryOpaqueCaptureAddress);

			return true;
		}

		//VK_KHR_deferred_host_operations- Requires support for Vulkan 1.0

		MIRU_PFN_DEFINITION_NULL(vkCreateDeferredOperationKHR);
		MIRU_PFN_DEFINITION_NULL(vkDestroyDeferredOperationKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetDeferredOperationMaxConcurrencyKHR);
		MIRU_PFN_DEFINITION_NULL(vkGetDeferredOperationResultKHR);
		MIRU_PFN_DEFINITION_NULL(vkDeferredOperationJoinKHR);

		inline bool LoadPFN_VK_KHR_deferred_host_operations(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCreateDeferredOperationKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkDestroyDeferredOperationKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeferredOperationMaxConcurrencyKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeferredOperationResultKHR);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkDeferredOperationJoinKHR);

			return true;
		}

		//VK_KHR_timeline_semaphore - Promoted to Vulkan 1.2
#if !defined(MIRU_VK_API_VERSION_1_2)
		MIRU_PFN_DEFINITION_LOAD(vkGetSemaphoreCounterValue);
		MIRU_PFN_DEFINITION_LOAD(vkSignalSemaphore);
		MIRU_PFN_DEFINITION_LOAD(vkWaitSemaphores);
#else
		MIRU_PFN_DEFINITION_NULL(vkGetSemaphoreCounterValue);
		MIRU_PFN_DEFINITION_NULL(vkSignalSemaphore);
		MIRU_PFN_DEFINITION_NULL(vkWaitSemaphores);
#endif
		inline bool LoadPFN_VK_KHR_timeline_semaphore(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetSemaphoreCounterValue);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkSignalSemaphore);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkWaitSemaphores);

			return true;
		}

		//VK_KHR_synchronization2 - Promoted to Vulkan 1.3
#if !defined(MIRU_VK_API_VERSION_1_3)
		MIRU_PFN_DEFINITION_LOAD(vkCmdPipelineBarrier2);
		MIRU_PFN_DEFINITION_LOAD(vkCmdResetEvent2);
		MIRU_PFN_DEFINITION_LOAD(vkCmdSetEvent2);
		MIRU_PFN_DEFINITION_LOAD(vkCmdWaitEvents2);
		MIRU_PFN_DEFINITION_LOAD(vkCmdWriteTimestamp2);
		MIRU_PFN_DEFINITION_LOAD(vkQueueSubmit2);
#else
		MIRU_PFN_DEFINITION_NULL(vkCmdPipelineBarrier2);
		MIRU_PFN_DEFINITION_NULL(vkCmdResetEvent2);
		MIRU_PFN_DEFINITION_NULL(vkCmdSetEvent2);
		MIRU_PFN_DEFINITION_NULL(vkCmdWaitEvents2);
		MIRU_PFN_DEFINITION_NULL(vkCmdWriteTimestamp2);
		MIRU_PFN_DEFINITION_NULL(vkQueueSubmit2);
#endif
		inline bool LoadPFN_VK_KHR_synchronization2(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdPipelineBarrier2);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdResetEvent2);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdSetEvent2);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdWaitEvents2);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdWriteTimestamp2);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkQueueSubmit2);

			return true;
		}

		//VK_EXT_mesh_shader - Promoted to Vulkan 1.3
		MIRU_PFN_DEFINITION_NULL(vkCmdDrawMeshTasksEXT);
		MIRU_PFN_DEFINITION_NULL(vkCmdDrawMeshTasksIndirectCountEXT);
		MIRU_PFN_DEFINITION_NULL(vkCmdDrawMeshTasksIndirectEXT);

		inline bool LoadPFN_VK_EXT_mesh_shader(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdDrawMeshTasksEXT);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdDrawMeshTasksIndirectCountEXT);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdDrawMeshTasksIndirectEXT);

			return true;
		}

		//VK_KHR_dynamic_rendering - Promoted to Vulkan 1.3
#if !defined(MIRU_VK_API_VERSION_1_3)
		MIRU_PFN_DEFINITION_LOAD(vkCmdBeginRendering);
		MIRU_PFN_DEFINITION_LOAD(vkCmdEndRendering);
#else
		MIRU_PFN_DEFINITION_NULL(vkCmdBeginRendering);
		MIRU_PFN_DEFINITION_NULL(vkCmdEndRendering);
#endif
		inline bool LoadPFN_VK_KHR_dynamic_rendering(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdBeginRendering);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdEndRendering);

			return true;
		}
	}
}

//MIRU SetName
#include "vulkan/VK_SetName.h"