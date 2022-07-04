#pragma once
#if defined(MIRU_VULKAN)
//Platform usage
#if defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR

//Vulkan Header
#include "vulkan/vulkan.h"

#if !defined(VK_VERSION_1_3)
#error Please update Vulkan SDK to version 1.3 or later. Download from https://vulkan.lunarg.com/sdk/home.
#endif

//VulkanMemoryAllocator
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"

//SPIR-V Cross Core
#include "SPIRV-Cross/spirv_cross.hpp"

#elif defined(__ANDROID__)
#if !defined( VK_USE_PLATFORM_ANDROID_KHR)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

//Header and Library
#include "vulkan_wrapper.h"

//Spirv-cross Header and Library
#include "spirv_cross/Include/spirv_cross.hpp"

#endif

//Vulkan Extension
#define MIRU_PFN_DEFINITION(fn) inline PFN_##fn fn = ::fn
#define MIRU_PFN_DEFINITION_NO_INIT(fn) inline PFN_##fn fn
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

		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceFeatures2);
		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceFormatProperties2);
		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceImageFormatProperties2);
		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceMemoryProperties2);
		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceProperties2);
		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceQueueFamilyProperties2);
		MIRU_PFN_DEFINITION(vkGetPhysicalDeviceSparseImageFormatProperties2);

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

		MIRU_PFN_DEFINITION_NO_INIT(vkSetDebugUtilsObjectNameEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkSetDebugUtilsObjectTagEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkQueueBeginDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkQueueEndDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkQueueInsertDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdBeginDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdEndDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdInsertDebugUtilsLabelEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkCreateDebugUtilsMessengerEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkDestroyDebugUtilsMessengerEXT);
		MIRU_PFN_DEFINITION_NO_INIT(vkSubmitDebugUtilsMessageEXT);

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

		MIRU_PFN_DEFINITION_NO_INIT(vkCmdTraceRaysKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCreateRayTracingPipelinesKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdTraceRaysIndirectKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetRayTracingShaderGroupStackSizeKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdSetRayTracingPipelineStackSizeKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetRayTracingShaderGroupHandlesKHR);

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

		MIRU_PFN_DEFINITION_NO_INIT(vkCreateAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkDestroyAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdBuildAccelerationStructuresKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdBuildAccelerationStructuresIndirectKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkBuildAccelerationStructuresKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCopyAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCopyAccelerationStructureToMemoryKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCopyMemoryToAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkWriteAccelerationStructuresPropertiesKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdCopyAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdCopyAccelerationStructureToMemoryKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdCopyMemoryToAccelerationStructureKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetAccelerationStructureDeviceAddressKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkCmdWriteAccelerationStructuresPropertiesKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetDeviceAccelerationStructureCompatibilityKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetAccelerationStructureBuildSizesKHR);

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

		MIRU_PFN_DEFINITION(vkGetBufferDeviceAddress);
		MIRU_PFN_DEFINITION(vkGetBufferOpaqueCaptureAddress);
		MIRU_PFN_DEFINITION(vkGetDeviceMemoryOpaqueCaptureAddress);

		inline bool LoadPFN_VK_KHR_buffer_device_address(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetBufferDeviceAddress);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetBufferOpaqueCaptureAddress);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeviceMemoryOpaqueCaptureAddress);

			return true;
		}

		//VK_KHR_deferred_host_operations- Requires support for Vulkan 1.0

		MIRU_PFN_DEFINITION_NO_INIT(vkCreateDeferredOperationKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkDestroyDeferredOperationKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetDeferredOperationMaxConcurrencyKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkGetDeferredOperationResultKHR);
		MIRU_PFN_DEFINITION_NO_INIT(vkDeferredOperationJoinKHR);

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

		MIRU_PFN_DEFINITION(vkGetSemaphoreCounterValue);
		MIRU_PFN_DEFINITION(vkSignalSemaphore);
		MIRU_PFN_DEFINITION(vkWaitSemaphores);

		inline bool LoadPFN_VK_KHR_timeline_semaphore(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetSemaphoreCounterValue);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkSignalSemaphore);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkWaitSemaphores);

			return true;
		}

		//VK_KHR_synchronization2 - Promoted to Vulkan 1.3

		MIRU_PFN_DEFINITION(vkCmdPipelineBarrier2);
		MIRU_PFN_DEFINITION(vkCmdResetEvent2);
		MIRU_PFN_DEFINITION(vkCmdSetEvent2);
		MIRU_PFN_DEFINITION(vkCmdWaitEvents2);
		MIRU_PFN_DEFINITION(vkCmdWriteTimestamp2);
		MIRU_PFN_DEFINITION(vkQueueSubmit2);

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

		//VK_KHR_dynamic_rendering - Promoted to Vulkan 1.3

		MIRU_PFN_DEFINITION(vkCmdBeginRendering);
		MIRU_PFN_DEFINITION(vkCmdEndRendering);

		inline bool LoadPFN_VK_KHR_dynamic_rendering(VkDevice& device)
		{
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdBeginRendering);
			MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdEndRendering);

			return true;
		}
	}
}
#endif