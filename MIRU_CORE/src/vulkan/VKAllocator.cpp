#include "common.h"
#include "VKAllocator.h"
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

VkPhysicalDeviceProperties MemoryBlock::s_PhysicalDeviceProperties = {};
VkPhysicalDeviceMemoryProperties MemoryBlock::s_PhysicalDeviceMemoryProperties = {};
uint32_t MemoryBlock::s_MaxAllocations = 0;
uint32_t MemoryBlock::s_CurrentAllocations = 0;

MemoryBlock::MemoryBlock(MemoryBlock::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Device = *reinterpret_cast<VkDevice*>(m_CI.pContext->GetDevice());
	Ref<vulkan::Context> context = ref_cast<vulkan::Context>(m_CI.pContext);

	s_PhysicalDeviceProperties = context->m_PhysicalDevices.m_PhysicalDeviceProperties[0];
	s_PhysicalDeviceMemoryProperties = context->m_PhysicalDevices.m_PhysicalDeviceMemoryProperties[0];
	s_MaxAllocations = s_PhysicalDeviceProperties.limits.maxMemoryAllocationCount;
	
	if (s_CurrentAllocations + 1 > s_MaxAllocations)
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: Exceeded maximum allocation.");
	}
	else
		s_CurrentAllocations++;

	m_MemoryTypeIndex = GetMemoryTypeIndex(static_cast<VkMemoryPropertyFlags>(m_CI.properties));
	m_AI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	m_AI.pNext = nullptr;
	m_AI.allocationSize = static_cast<VkDeviceSize>(m_CI.blockSize);
	m_AI.memoryTypeIndex = m_MemoryTypeIndex;

	MIRU_ASSERT(vkAllocateMemory(m_Device, &m_AI, nullptr, &m_DeviceMemory), "ERROR: VULKAN: Failed to allocate Memory.");
	VKSetName<VkDeviceMemory>(m_Device, (uint64_t)m_DeviceMemory, m_CI.debugName);

	s_MemoryBlocks.push_back(this);
	s_AllocatedResources[this];
}

MemoryBlock::~MemoryBlock()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkFreeMemory(m_Device, m_DeviceMemory, nullptr);
}

bool MemoryBlock::AddResource(crossplatform::Resource& resource)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_Device != *reinterpret_cast<VkDevice*>(resource.device))
		return false;

	if (!ResourceBackable(resource))
		return false;

	//TODO Re-enable this. Issue with GetMemoryPropertyFlag()
	/*if (m_MemoryTypeIndex != GetMemoryTypeIndex(GetMemoryPropertyFlag(resource.type, resource.usage)))
		return false;*/

	resource.memoryBlock = (uint64_t)m_DeviceMemory;
	resource.id = GenerateURID();
	s_AllocatedResources[this][resource.id] = resource;
	CalculateOffsets();
	resource = s_AllocatedResources[this][resource.id];

	if (resource.type == crossplatform::Resource::Type::BUFFER)
		MIRU_ASSERT(vkBindBufferMemory(m_Device, (VkBuffer)resource.resource, m_DeviceMemory, resource.offset), "ERROR: VULKAN: Failed to bind Buffer.");
	if (resource.type == crossplatform::Resource::Type::IMAGE)
		MIRU_ASSERT(vkBindImageMemory(m_Device, (VkImage)resource.resource, m_DeviceMemory, resource.offset), "ERROR: VULKAN: Failed to bind Image.");

	return true;
}

void MemoryBlock::RemoveResource(uint64_t id)
{
	MIRU_CPU_PROFILE_FUNCTION();

	s_AllocatedResources[this].erase(id);
}

void MemoryBlock::SubmitData(const crossplatform::Resource& resource, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if ((m_MemoryTypeIndex == GetMemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		|| m_MemoryTypeIndex == GetMemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		&& data)
	{
		void* mappedData;
		MIRU_ASSERT(vkMapMemory(m_Device, m_DeviceMemory, resource.offset, resource.size, 0, &mappedData), "ERROR: VULKAN: Can not map resource.");
		memcpy(mappedData, data, static_cast<size_t>(resource.size));
		vkUnmapMemory(m_Device, m_DeviceMemory);
	}
}

VkMemoryPropertyFlags MemoryBlock::GetMemoryPropertyFlag(crossplatform::Resource::Type type, uint32_t usage)
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

uint32_t MemoryBlock::GetMemoryTypeIndex(VkMemoryPropertyFlags properties)
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
uint32_t MemoryBlock::GetQueueFamilyIndex(VkQueueFlagBits queueType)
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

	uint32_t i = 0, queueIndex = 0;
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
}