#define VMA_IMPLEMENTATION
#include "VKAllocator.h"
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

Allocator::Allocator(Allocator::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Device = *reinterpret_cast<VkDevice*>(m_CI.context->GetDevice());
	const ContextRef& context = ref_cast<Context>(m_CI.context);
	
	bool buffer_device_address = false;
	buffer_device_address |= context->IsActive(context->m_ActiveDeviceExtensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	buffer_device_address |= (context->m_AI.apiVersion >= VK_API_VERSION_1_2);

	m_AI.flags = buffer_device_address ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
	m_AI.physicalDevice = context->m_PhysicalDevices.m_PDIs[0].m_PhysicalDevice;
	m_AI.device = m_Device;
	m_AI.preferredLargeHeapBlockSize = static_cast<VkDeviceSize>(m_CI.blockSize);
	m_AI.pAllocationCallbacks = nullptr;
	m_AI.pDeviceMemoryCallbacks = nullptr;
	m_AI.pHeapSizeLimit = nullptr;
	m_AI.pVulkanFunctions = nullptr;
	m_AI.instance = context->m_Instance;
	m_AI.vulkanApiVersion = 0; // context->m_AI.apiVersion;
	m_AI.pTypeExternalMemoryHandleTypes = nullptr;
	
	MIRU_FATAL(vmaCreateAllocator(&m_AI, &m_Allocator), "ERROR: VULKAN: Failed to create Allocator.");
}

Allocator::~Allocator()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vmaDestroyAllocator(m_Allocator);
}

void* Allocator::GetNativeAllocator()
{
	return reinterpret_cast<void*>(&m_Allocator);
}

void Allocator::SubmitData(const base::Allocation& allocation, size_t offset, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (allocation.nativeAllocation && size > 0 && data)
	{
		const VmaAllocation& vmaAllocation = *reinterpret_cast<VmaAllocation*>(allocation.nativeAllocation);

		const bool& hostVisible = arc::BitwiseCheck(static_cast<VkMemoryPropertyFlagBits>(m_CI.properties), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		const bool& hostCoherent = arc::BitwiseCheck(static_cast<VkMemoryPropertyFlagBits>(m_CI.properties), VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (hostVisible)
		{
			void* mappedData;
			MIRU_FATAL(vmaMapMemory(m_Allocator, vmaAllocation, &mappedData), "ERROR: VULKAN: Can not map resource.");
			memcpy(reinterpret_cast<char*>(mappedData) + offset, data, size);
			if (!hostCoherent)
				vmaFlushAllocation(m_Allocator, vmaAllocation, offset, size);

			vmaUnmapMemory(m_Allocator, vmaAllocation);
		}
	}
}

void Allocator::AccessData(const base::Allocation& allocation, size_t offset, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (allocation.nativeAllocation && size > 0 && data)
	{
		const VmaAllocation& vmaAllocation = *reinterpret_cast<VmaAllocation*>(allocation.nativeAllocation);
		
		const bool& hostVisible = arc::BitwiseCheck(static_cast<VkMemoryPropertyFlagBits>(m_CI.properties), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		const bool& hostCoherent = arc::BitwiseCheck(static_cast<VkMemoryPropertyFlagBits>(m_CI.properties), VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (hostVisible)
		{
			void* mappedData;
			MIRU_FATAL(vmaMapMemory(m_Allocator, vmaAllocation, &mappedData), "ERROR: VULKAN: Can not map resource.");
			if (!hostCoherent)
				vmaInvalidateAllocation(m_Allocator, vmaAllocation, offset, size);
			
			memcpy(data, reinterpret_cast<char*>(mappedData) + offset, size);
			vmaUnmapMemory(m_Allocator, vmaAllocation);
		}
	}
}