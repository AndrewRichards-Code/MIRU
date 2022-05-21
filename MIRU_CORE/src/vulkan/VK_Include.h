#pragma once
#if defined(MIRU_VULKAN)
//Platform usage
#if defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR

//Vulkan Header
#include "vulkan/vulkan.h"

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
#define MIRU_PFN_VK_GET_INSTANCE_PROC_ADDR(fn) fn = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn); if(!fn) { return false; }
#define MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(fn) fn = (PFN_##fn)vkGetDeviceProcAddr(device, #fn); if(!fn) { return false; }

namespace miru
{
namespace vulkan
{
	//Current Extensions

#if defined(VK_KHR_acceleration_structure)
	inline PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	inline PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	inline PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	inline PFN_vkCmdBuildAccelerationStructuresIndirectKHR vkCmdBuildAccelerationStructuresIndirectKHR;
	inline PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
	inline PFN_vkCopyAccelerationStructureKHR vkCopyAccelerationStructureKHR;
	inline PFN_vkCopyAccelerationStructureToMemoryKHR vkCopyAccelerationStructureToMemoryKHR;
	inline PFN_vkCopyMemoryToAccelerationStructureKHR vkCopyMemoryToAccelerationStructureKHR;
	inline PFN_vkWriteAccelerationStructuresPropertiesKHR vkWriteAccelerationStructuresPropertiesKHR;
	inline PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR;
	inline PFN_vkCmdCopyAccelerationStructureToMemoryKHR vkCmdCopyAccelerationStructureToMemoryKHR;
	inline PFN_vkCmdCopyMemoryToAccelerationStructureKHR vkCmdCopyMemoryToAccelerationStructureKHR;
	inline PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	inline PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR;
	inline PFN_vkGetDeviceAccelerationStructureCompatibilityKHR vkGetDeviceAccelerationStructureCompatibilityKHR;
	inline PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;

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
#endif

#if defined(VK_KHR_buffer_device_address)
	inline PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	inline PFN_vkGetBufferOpaqueCaptureAddressKHR vkGetBufferOpaqueCaptureAddressKHR;
	inline PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR;

	inline bool LoadPFN_VK_KHR_buffer_device_address(VkDevice& device)
	{
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetBufferDeviceAddressKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetBufferOpaqueCaptureAddressKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeviceMemoryOpaqueCaptureAddressKHR);

		return true;
	}
#endif

#if defined(VK_KHR_deferred_host_operations)
	inline PFN_vkCreateDeferredOperationKHR vkCreateDeferredOperationKHR;
	inline PFN_vkDestroyDeferredOperationKHR vkDestroyDeferredOperationKHR;
	inline PFN_vkGetDeferredOperationMaxConcurrencyKHR vkGetDeferredOperationMaxConcurrencyKHR;
	inline PFN_vkGetDeferredOperationResultKHR vkGetDeferredOperationResultKHR;
	inline PFN_vkDeferredOperationJoinKHR vkDeferredOperationJoinKHR;

	inline bool LoadPFN_VK_KHR_deferred_host_operations(VkDevice& device)
	{
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCreateDeferredOperationKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkDestroyDeferredOperationKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeferredOperationMaxConcurrencyKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetDeferredOperationResultKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkDeferredOperationJoinKHR);

		return true;
	}
#endif

#if defined(VK_KHR_ray_tracing_pipeline)
	inline PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	inline PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
	inline PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR vkGetRayTracingCaptureReplayShaderGroupHandlesKHR;
	inline PFN_vkCmdTraceRaysIndirectKHR vkCmdTraceRaysIndirectKHR;
	inline PFN_vkGetRayTracingShaderGroupStackSizeKHR vkGetRayTracingShaderGroupStackSizeKHR;
	inline PFN_vkCmdSetRayTracingPipelineStackSizeKHR vkCmdSetRayTracingPipelineStackSizeKHR;
	inline PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;

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
#endif

#if defined(VK_EXT_debug_utils)
	inline PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	inline PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTagEXT;
	inline PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
	inline PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
	inline PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT;
	inline PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	inline PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	inline PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
	inline PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	inline PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	inline PFN_vkSubmitDebugUtilsMessageEXT vkSubmitDebugUtilsMessageEXT;

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
#endif

#if defined(VK_KHR_timeline_semaphore)
	inline PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR;
	inline PFN_vkSignalSemaphoreKHR vkSignalSemaphoreKHR;
	inline PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR;

	inline bool LoadPFN_VK_KHR_timeline_semaphore(VkDevice& device)
	{
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkGetSemaphoreCounterValueKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkSignalSemaphoreKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkWaitSemaphoresKHR);

		return true;
	}
#endif

#if defined(VK_KHR_synchronization2)
	inline PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;
	inline PFN_vkCmdResetEvent2KHR vkCmdResetEvent2KHR;
	inline PFN_vkCmdSetEvent2KHR vkCmdSetEvent2KHR;
	inline PFN_vkCmdWaitEvents2KHR vkCmdWaitEvents2KHR;
	inline PFN_vkCmdWriteTimestamp2KHR vkCmdWriteTimestamp2KHR;
	inline PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;

	inline bool LoadPFN_VK_KHR_synchronization2(VkDevice& device)
	{
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdPipelineBarrier2KHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdResetEvent2KHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdSetEvent2KHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdWaitEvents2KHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdWriteTimestamp2KHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkQueueSubmit2KHR);

		return true;
	}
#endif

#if defined(VK_KHR_dynamic_rendering)
	inline PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
	inline PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;

	inline bool LoadPFN_VK_KHR_dynamic_rendering(VkDevice& device)
	{
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdBeginRenderingKHR);
		MIRU_PFN_VK_GET_DEVICE_PROC_ADDR(vkCmdEndRenderingKHR);

		return true;
	}
#endif

}
}
#endif