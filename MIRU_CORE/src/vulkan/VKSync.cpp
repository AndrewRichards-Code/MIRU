#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKSync.h"

#include "VKBuffer.h"
#include "VKImage.h"

using namespace miru;
using namespace vulkan;

//Fence
Fence::Fence(Fence::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	m_FenceCI.pNext = nullptr;
	m_FenceCI.flags = m_CI.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	MIRU_ASSERT(vkCreateFence(m_Device, &m_FenceCI, nullptr, &m_Fence), "ERROR: VULKAN: Failed to a create Fence.");
	VKSetName<VkFence>(m_Device, (uint64_t)m_Fence, m_CI.debugName);
}

Fence::~Fence()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyFence(m_Device, m_Fence, nullptr);
}

void Fence::Reset()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_ASSERT(vkResetFences(m_Device, 1, &m_Fence), "ERROR: VULKAN: Failed to reset Fence.");
}

bool Fence::GetStatus()
{
	MIRU_CPU_PROFILE_FUNCTION();

	VkResult result = vkGetFenceStatus(m_Device, m_Fence);
	if (result == VK_SUCCESS)
		return true;
	else if (result == VK_NOT_READY)
		return false;
	else
	{
		MIRU_ASSERT(result, "ERROR: VULKAN: Failed to get status of Fence.");
		return false;
	}
}

bool Fence::Wait()
{
	MIRU_CPU_PROFILE_FUNCTION();

	VkResult result = vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, m_CI.timeout);
	if (result == VK_SUCCESS)
		return true;
	else if (result == VK_TIMEOUT)
		return false;
	else
	{
		MIRU_ASSERT(result, "ERROR: VULKAN: Failed to wait for Fence.");
		return false;
	}
}

//Semaphore
Semaphore::Semaphore(Semaphore::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	m_SemaphoreCI.pNext = nullptr;
	m_SemaphoreCI.flags = 0;

	MIRU_ASSERT(vkCreateSemaphore(m_Device, &m_SemaphoreCI, nullptr, &m_Semaphore), "ERROR: VULKAN: Failed to a create Semaphore.");
	VKSetName<VkSemaphore>(m_Device, (uint64_t)m_Semaphore, m_CI.debugName);
}

Semaphore::~Semaphore()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroySemaphore(m_Device, m_Semaphore, nullptr);
}

//Event
Event::Event(Event::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_EventCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	m_EventCI.pNext = nullptr;
	m_EventCI.flags = 0;

	MIRU_ASSERT(vkCreateEvent(m_Device, &m_EventCI, nullptr, &m_Event), "ERROR: VULKAN: Failed to a create Event.");
	VKSetName<VkEvent>(m_Device, (uint64_t)m_Event, m_CI.debugName);
}

Event::~Event()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyEvent(m_Device, m_Event, nullptr);
}

void Event::Set()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_ASSERT(vkSetEvent(m_Device, m_Event), "ERROR: VULKAN: Failed to set Event.");
}

void Event::Reset()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_ASSERT(vkResetEvent(m_Device, m_Event), "ERROR: VULKAN: Failed to reset Event.");
}

bool Event::GetStatus()
{
	MIRU_CPU_PROFILE_FUNCTION();

	VkResult result = vkGetEventStatus(m_Device, m_Event);
	if (result == VK_EVENT_SET)
		return false;
	else if (result == VK_EVENT_RESET)
		return true;
	else if (result)
	{
		MIRU_ASSERT(result, "ERROR: VULKAN: Failed to get status of Event.");
		return true;
	}
	else
		return true;
}

//Barrier
Barrier::Barrier(Barrier::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	if (m_CI.type == Barrier::Type::MEMORY)
	{

		m_MB.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		m_MB.pNext = nullptr;
		m_MB.srcAccessMask = static_cast<VkAccessFlagBits>(m_CI.srcAccess);
		m_MB.dstAccessMask = static_cast<VkAccessFlagBits>(m_CI.dstAccess);
	}
	else if (m_CI.type == Barrier::Type::BUFFER)
	{
		m_BMB.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		m_BMB.pNext = nullptr;
		m_BMB.srcAccessMask = static_cast<VkAccessFlagBits>(m_CI.srcAccess);
		m_BMB.dstAccessMask = static_cast<VkAccessFlagBits>(m_CI.dstAccess);
		m_BMB.srcQueueFamilyIndex = m_CI.srcQueueFamilyIndex;
		m_BMB.dstQueueFamilyIndex = m_CI.dstQueueFamilyIndex;
		m_BMB.buffer = ref_cast<vulkan::Buffer>(m_CI.pBuffer)->m_Buffer;
		m_BMB.offset = m_CI.offset;
		m_BMB.size = m_CI.size;
	}
	else if (m_CI.type == Barrier::Type::IMAGE)
	{
		m_IMB.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		m_IMB.pNext = nullptr;
		m_IMB.srcAccessMask = static_cast<VkAccessFlagBits>(m_CI.srcAccess);
		m_IMB.dstAccessMask = static_cast<VkAccessFlagBits>(m_CI.dstAccess);
		m_IMB.srcQueueFamilyIndex = m_CI.srcQueueFamilyIndex;
		m_IMB.dstQueueFamilyIndex = m_CI.dstQueueFamilyIndex;
		m_IMB.oldLayout = static_cast<VkImageLayout>(m_CI.oldLayout);
		m_IMB.newLayout = static_cast<VkImageLayout>(m_CI.newLayout);
		m_IMB.image = ref_cast<vulkan::Image>(m_CI.pImage)->m_Image;
		m_IMB.subresourceRange.aspectMask = static_cast<VkImageAspectFlagBits>(m_CI.subresourceRange.aspect);
		m_IMB.subresourceRange.baseMipLevel = m_CI.subresourceRange.baseMipLevel;
		m_IMB.subresourceRange.levelCount = m_CI.subresourceRange.mipLevelCount;
		m_IMB.subresourceRange.baseArrayLayer = m_CI.subresourceRange.baseArrayLayer;
		m_IMB.subresourceRange.layerCount = m_CI.subresourceRange.arrayLayerCount;
	}
	else
		return;
}

Barrier::~Barrier()
{
	MIRU_CPU_PROFILE_FUNCTION();

}
#endif