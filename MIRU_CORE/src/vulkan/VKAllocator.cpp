#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKAllocator.h"
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

Allocator::Allocator(Allocator::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Device = *reinterpret_cast<VkDevice*>(m_CI.pContext->GetDevice());
	const Ref<vulkan::Context>& context = ref_cast<vulkan::Context>(m_CI.pContext);
	
	bool buffer_device_address = false;
	buffer_device_address |= context->IsActive(context->m_ActiveDeviceExtensions, "VK_KHR_buffer_device_address");
	buffer_device_address |= (context->m_AI.apiVersion >= VK_API_VERSION_1_2);

	m_AI.flags = buffer_device_address ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
	m_AI.physicalDevice = context->m_PhysicalDevices.m_PhysicalDevices[0];
	m_AI.device = m_Device;
	m_AI.preferredLargeHeapBlockSize = static_cast<VkDeviceSize>(m_CI.blockSize);
	m_AI.pAllocationCallbacks = nullptr;
	m_AI.pDeviceMemoryCallbacks = nullptr;
	m_AI.frameInUseCount = 0;
	m_AI.pHeapSizeLimit = nullptr;
	m_AI.pVulkanFunctions = nullptr;
	m_AI.pRecordSettings = nullptr;
	m_AI.instance = context->m_Instance;
	m_AI.vulkanApiVersion = 0; // context->m_AI.apiVersion;
	
	MIRU_ASSERT(vmaCreateAllocator(&m_AI, &m_Allocator), "ERROR: VULKAN: Failed to create Allocator.");
	//VKSetName<VkDeviceMemory>(m_Device, (uint64_t)m_DeviceMemory, m_CI.debugName);
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

void Allocator::SubmitData(const crossplatform::Allocation& allocation, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (allocation.nativeAllocation && size > 0 && data)
	{
		const VmaAllocation& vmaAllocation = allocation.GetVmaAllocation();

		bool hostVisible = static_cast<VkMemoryPropertyFlags>(m_CI.properties) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		bool hostCoherent = static_cast<VkMemoryPropertyFlags>(m_CI.properties) & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if (hostVisible)
		{
			void* mappedData;
			MIRU_ASSERT(vmaMapMemory(m_Allocator, vmaAllocation, &mappedData), "ERROR: VULKAN: Can not map resource.");
			memcpy(mappedData, data, size);
			if (!hostCoherent)
				vmaFlushAllocation(m_Allocator, vmaAllocation, 0, VK_WHOLE_SIZE);

			vmaUnmapMemory(m_Allocator, vmaAllocation);
		}
	}
}

void Allocator::AccessData(const crossplatform::Allocation& allocation, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (allocation.nativeAllocation && size > 0 && data)
	{
		const VmaAllocation& vmaAllocation = allocation.GetVmaAllocation();
		
		bool hostVisible = static_cast<VkMemoryPropertyFlags>(m_CI.properties) & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		bool hostCoherent = static_cast<VkMemoryPropertyFlags>(m_CI.properties) & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if (hostVisible)
		{
			void* mappedData;
			MIRU_ASSERT(vmaMapMemory(m_Allocator, vmaAllocation, &mappedData), "ERROR: VULKAN: Can not map resource.");
			if (!hostCoherent)
				vmaInvalidateAllocation(m_Allocator, vmaAllocation, 0, VK_WHOLE_SIZE);
			
			memcpy(data, mappedData, size);
			vmaUnmapMemory(m_Allocator, vmaAllocation);
		}
	}
}

/*VkMemoryPropertyFlags Allocator::GetMemoryPropertyFlag(crossplatform::Resource::Type type, uint32_t usage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	VkMemoryPropertyFlags flags = 0;

	if (type == crossplatform::Resource::Type::BUFFER)
	{
		VkBufferUsageFlags _usage = usage;
		if(_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			flags += VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if (_usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT
			|| _usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
			|| _usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT)
			flags += VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		return flags;
	}
	else if (type == crossplatform::Resource::Type::IMAGE)
	{
		VkImageUsageFlags _usage = usage;
		if (_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			flags += VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if(_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT
		|| _usage & VK_IMAGE_USAGE_SAMPLED_BIT
		|| _usage & VK_IMAGE_USAGE_STORAGE_BIT
		|| _usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		|| _usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		|| _usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
		|| _usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
			flags += VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		
		return flags;
	}
	else
		return flags;
}

uint32_t Allocator::GetMemoryTypeIndex(VkMemoryPropertyFlags properties)
{
	MIRU_CPU_PROFILE_FUNCTION();

	uint32_t memoryTypeIndex = 0;
	for (uint32_t i = 0; i < s_PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((s_PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			memoryTypeIndex = i;
			break;
		}
	}
	
	return memoryTypeIndex;
}

//Unused but very useful!
uint32_t Allocator::GetQueueFamilyIndex(VkQueueFlagBits queueType)
{
	MIRU_CPU_PROFILE_FUNCTION();

	const std::vector<VkQueueFamilyProperties>& queueFamilyProperties = ref_cast<Context>(m_CI.pContext)->m_QueueFamilyProperties;
	
	uint32_t nextPowerOfTwo = static_cast<uint32_t>(pow(2, ceil(log((uint32_t)queueType) / log(2))));
	bool IsSingleType = (nextPowerOfTwo == (uint32_t)queueType ? true : false);
	bool HasGraphicsBit = queueType & VK_QUEUE_GRAPHICS_BIT;
	uint32_t boundedInvQueueType = 0;
	if (IsSingleType)
		boundedInvQueueType = (uint32_t)queueType - 1;
	else
		boundedInvQueueType = (~queueType & 0x0000000F) - (0x0000000F - nextPowerOfTwo) - 1;

	uint32_t i = 0;
	for (auto& queueFamilyProperty : queueFamilyProperties)
	{
		if ((queueFamilyProperty.queueFlags & queueType) == queueType)
		{
			if (!(queueFamilyProperty.queueFlags & boundedInvQueueType) || HasGraphicsBit)
				return i;
		}
		i++;
	}
	return 0;
}*/
#endif