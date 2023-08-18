#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKBuffer.h"
#include "base/Allocator.h"

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
	m_VmaACI.requiredFlags = static_cast<VkMemoryPropertyFlags>(m_CI.allocator->GetCreateInfo().properties);
	m_VmaACI.preferredFlags = 0;
	m_VmaACI.memoryTypeBits = 0;
	m_VmaACI.pool = VK_NULL_HANDLE;
	m_VmaACI.pUserData = nullptr;

	MIRU_ASSERT(vmaCreateBuffer(m_CI.allocator->GetVmaAllocator(), &m_BufferCI, &m_VmaACI, &m_Buffer, &m_VmaAllocation, &m_VmaAI), "ERROR: VULKAN: Failed to create Buffer.");
	VKSetName<VkBuffer>(m_Device, m_Buffer, m_CI.debugName);

	m_Allocation.nativeAllocation = (base::NativeAllocation)&m_VmaAllocation;
	m_Allocation.width = 0;
	m_Allocation.height = 0;
	m_Allocation.rowPitch = 0;
	m_Allocation.rowPadding = 0;

	if (m_CI.data)
	{
		m_CI.allocator->SubmitData(m_Allocation, 0, m_CI.size, m_CI.data);
	}
}

Buffer::~Buffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vmaDestroyBuffer(m_CI.allocator->GetVmaAllocator(), m_Buffer, m_VmaAllocation);
}

BufferView::BufferView(CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_BufferViewCI.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	m_BufferViewCI.pNext = nullptr;
	m_BufferViewCI.flags = 0;
	m_BufferViewCI.buffer = ref_cast<Buffer>(m_CI.buffer)->m_Buffer;
	m_BufferViewCI.format = VK_FORMAT_R8_UINT;
	m_BufferViewCI.offset = m_CI.offset;
	m_BufferViewCI.range = m_CI.size;

	if (m_CI.buffer->GetCreateInfo().usage == Buffer::UsageBit::UNIFORM_TEXEL_BIT
		|| m_CI.buffer->GetCreateInfo().usage == Buffer::UsageBit::STORAGE_TEXEL_BIT) // These are the only valid usage to create a VkBufferView proper.
	{
		MIRU_ASSERT(vkCreateBufferView(m_Device, &m_BufferViewCI, nullptr, &m_BufferView), "ERROR: VULKAN: Failed to create BufferView.");
	}
}

BufferView::~BufferView()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_CI.buffer->GetCreateInfo().usage == Buffer::UsageBit::UNIFORM_TEXEL_BIT
		|| m_CI.buffer->GetCreateInfo().usage == Buffer::UsageBit::STORAGE_TEXEL_BIT) // These are the only valid usage to create a VkBufferView proper.
	{
		vkDestroyBufferView(m_Device, m_BufferView, nullptr);
	}
}
#endif