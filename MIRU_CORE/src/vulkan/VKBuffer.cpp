#include "common.h"
#include "VKBuffer.h"
#include "crossplatform/Allocator.h"

using namespace miru;
using namespace vulkan;

Buffer::Buffer(Buffer::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;
	
	m_BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	m_BufferCI.pNext = nullptr;
	m_BufferCI.flags = 0;
	m_BufferCI.size = static_cast<VkDeviceSize>(m_CI.size);
	m_BufferCI.usage = static_cast<VkBufferUsageFlags>(m_CI.usage);
	m_BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	m_BufferCI.queueFamilyIndexCount = 0;
	m_BufferCI.pQueueFamilyIndices = nullptr;

	MIRU_ASSERT(vkCreateBuffer(m_Device, &m_BufferCI, nullptr, &m_Buffer), "ERROR: VULKAN: Failed to create Buffer.");
	VKSetName<VkBuffer>(m_Device, (uint64_t)m_Buffer, m_CI.debugName);

	vkGetBufferMemoryRequirements(m_Device, m_Buffer, &m_MemoryRequirements);

	m_Resource.device = &m_Device;
	m_Resource.type = crossplatform::Resource::Type::BUFFER;
	m_Resource.resource = (uint64_t)m_Buffer;
	m_Resource.usage = static_cast<uint32_t>(m_CI.usage);
	m_Resource.size = m_MemoryRequirements.size;
	m_Resource.alignment = m_MemoryRequirements.alignment;
	
	m_CI.pMemoryBlock->AddResource(m_Resource);
	m_CI.pMemoryBlock->SubmitData(m_Resource, m_CI.size, m_CI.data);
}

Buffer::~Buffer()
{
	vkDestroyBuffer(m_Device, m_Buffer, nullptr);
	m_CI.pMemoryBlock->RemoveResource(m_Resource.id);
}

VkBufferUsageFlags Buffer::ToVKBufferType(Buffer::UsageBit type)
{
	switch (type)
	{
	case Buffer::UsageBit::TRANSFER_SRC:
		return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	case Buffer::UsageBit::TRANSFER_DST:
		return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	case Buffer::UsageBit::UNIFORM_TEXEL:
		return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	case Buffer::UsageBit::STORAGE_TEXEL:
		return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	case Buffer::UsageBit::UNIFORM:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case Buffer::UsageBit::STORAGE:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	case Buffer::UsageBit::INDEX:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case Buffer::UsageBit::VERTEX:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case Buffer::UsageBit::INDIRECT:
		return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	case Buffer::UsageBit::TRANSFORM_FEEDBACK:
		return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
	default:
		return 0;
	}
}

BufferView::BufferView(CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_BufferViewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	m_BufferViewCI.pNext = nullptr;
	m_BufferViewCI.flags = 0;
	m_BufferViewCI.buffer = ref_cast<Buffer>(m_CI.pBuffer)->m_Buffer;
	m_BufferViewCI.format = VK_FORMAT_R8_UINT;
	m_BufferViewCI.offset = m_CI.offset;
	m_BufferViewCI.range = m_CI.size;

	if (m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::UNIFORM_TEXEL
		|| m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::STORAGE_TEXEL) // These are the only valid usage to create a VkBufferView proper.
	{
		MIRU_ASSERT(vkCreateBufferView(m_Device, &m_BufferViewCI, nullptr, &m_BufferView), "ERROR: VULKAN: Failed to create BufferView.");
	}
}

BufferView::~BufferView()
{
	if (m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::UNIFORM_TEXEL
		|| m_CI.pBuffer->GetCreateInfo().usage == Buffer::UsageBit::STORAGE_TEXEL) // These are the only valid usage to create a VkBufferView proper.
	{
		vkDestroyBufferView(m_Device, m_BufferView, nullptr);
	}
}
