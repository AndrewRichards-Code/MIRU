#include "miru_core_common.h"
#include "VKBuffer.h"
#include "crossplatform/Allocator.h"

using namespace miru;
using namespace vulkan;

Buffer::Buffer(Buffer::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	
	m_BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_BufferCI.pNext = nullptr;
	m_BufferCI.flags = 0;
	m_BufferCI.size = static_cast<VkDeviceSize>(m_CI.size);
	m_BufferCI.usage = static_cast<VkBufferUsageFlags>(m_CI.usage);
	m_BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	m_BufferCI.queueFamilyIndexCount = 0;
	m_BufferCI.pQueueFamilyIndices = nullptr;

	m_VmaACI.flags = 0;
	m_VmaACI.usage = VMA_MEMORY_USAGE_UNKNOWN;
	m_VmaACI.requiredFlags = static_cast<VkMemoryPropertyFlags>(m_CI.pAllocator->GetCreateInfo().properties);
	m_VmaACI.preferredFlags = 0;
	m_VmaACI.memoryTypeBits = 0;
	m_VmaACI.pool = VK_NULL_HANDLE;
	m_VmaACI.pUserData = nullptr;

	MIRU_ASSERT(vmaCreateBuffer(m_CI.pAllocator->GetVmaAllocator(), &m_BufferCI, &m_VmaACI, &m_Buffer, &m_VmaAllocation, &m_VmaAI), "ERROR: VULKAN: Failed to create Buffer.");
	VKSetName<VkBuffer>(m_Device, (uint64_t)m_Buffer, m_CI.debugName);

	m_Allocation.nativeAllocation = (crossplatform::NativeAllocation)&m_VmaAllocation;
	m_Allocation.width = 0;
	m_Allocation.height = 0;
	m_Allocation.rowPitch = 0;
	m_Allocation.rowPadding = 0;

	if (m_CI.data)
	{
		m_CI.pAllocator->SubmitData(m_Allocation, m_CI.size, m_CI.data);
	}
}

Buffer::~Buffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vmaDestroyBuffer(m_CI.pAllocator->GetVmaAllocator(), m_Buffer, m_VmaAllocation);
}

VkBufferUsageFlags Buffer::ToVKBufferType(Buffer::UsageBit type)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (type)
	{
	case Buffer::UsageBit::TRANSFER_SRC_BIT:
		return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	case Buffer::UsageBit::TRANSFER_DST_BIT:
		return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	case Buffer::UsageBit::UNIFORM_TEXEL_BIT:
		return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	case Buffer::UsageBit::STORAGE_TEXEL_BIT:
		return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	case Buffer::UsageBit::UNIFORM_BIT:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case Buffer::UsageBit::STORAGE_BIT:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	case Buffer::UsageBit::INDEX_BIT:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case Buffer::UsageBit::VERTEX_BIT:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case Buffer::UsageBit::INDIRECT_BIT:
		return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	case Buffer::UsageBit::TRANSFORM_FEEDBACK_BIT:
		return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
	default:
		return 0;
	}
}

BufferView::BufferView(CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_BufferViewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	m_BufferViewCI.pNext = nullptr;
	m_BufferViewCI.flags = 0;
	m_BufferViewCI.buffer = ref_cast<Buffer>(m_CI.pBuffer)->m_Buffer;
	m_BufferViewCI.format = VK_FORMAT_R8_UINT;
	m_BufferViewCI.offset = m_CI.offset;
	m_BufferViewCI.range = m_CI.size;

	if (m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::UNIFORM_TEXEL_BIT
		|| m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::STORAGE_TEXEL_BIT) // These are the only valid usage to create a VkBufferView proper.
	{
		MIRU_ASSERT(vkCreateBufferView(m_Device, &m_BufferViewCI, nullptr, &m_BufferView), "ERROR: VULKAN: Failed to create BufferView.");
	}
}

BufferView::~BufferView()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::UNIFORM_TEXEL_BIT
		|| m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::STORAGE_TEXEL_BIT) // These are the only valid usage to create a VkBufferView proper.
	{
		vkDestroyBufferView(m_Device, m_BufferView, nullptr);
	}
}
