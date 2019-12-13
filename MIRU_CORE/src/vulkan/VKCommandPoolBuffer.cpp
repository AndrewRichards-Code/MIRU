#include "common.h"
#include "VKCommandPoolBuffer.h"

#include "VKContext.h"
#include "VKSwapchain.h"
#include "VKSync.h"
#include "VKImage.h"

using namespace miru;
using namespace vulkan;

//CmdPool
CommandPool::CommandPool(CommandPool::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->pContext->GetDevice()))
{
	m_CI = *pCreateInfo;

	m_CmdPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	m_CmdPoolCI.pNext = nullptr;
	m_CmdPoolCI.flags = static_cast<VkCommandPoolCreateFlags>(m_CI.flags);
	m_CmdPoolCI.queueFamilyIndex = m_CI.queueFamilyIndex;

	MIRU_ASSERT(vkCreateCommandPool(m_Device, &m_CmdPoolCI, nullptr, &m_CmdPool), "ERROR: VULKAN: Failed to create CommandPool.");
	VKSetName<VkCommandPool>(m_Device, (uint64_t)m_CmdPool, m_CI.debugName);
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(m_Device, m_CmdPool, nullptr);
}

void CommandPool::Trim()
{
	vkTrimCommandPool(m_Device, m_CmdPool, 0);
}

void CommandPool::Reset(bool releaseResources)
{
	MIRU_ASSERT(vkResetCommandPool(m_Device, m_CmdPool, releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0), "ERROR: VULKAN: Failed to reset CommandPool.");
}

//CmdBuffer
CommandBuffer::CommandBuffer(CommandBuffer::CreateInfo* pCreateInfo)
	:m_Device(std::dynamic_pointer_cast<CommandPool>(pCreateInfo->pCommandPool)->m_Device),
	m_CmdPool(std::dynamic_pointer_cast<CommandPool>(pCreateInfo->pCommandPool)->m_CmdPool)
{
	m_CI = *pCreateInfo;

	m_CmdBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	m_CmdBufferAI.pNext = nullptr;
	m_CmdBufferAI.commandPool = m_CmdPool;
	m_CmdBufferAI.level = static_cast<VkCommandBufferLevel>(m_CI.level);
	m_CmdBufferAI.commandBufferCount = m_CI.commandBufferCount;

	m_CmdBuffers.resize(m_CI.commandBufferCount);
	MIRU_ASSERT(vkAllocateCommandBuffers(m_Device, &m_CmdBufferAI, m_CmdBuffers.data()), "ERROR: VULKAN: Failed to allocate CommandBuffers.");

	size_t i = 0;
	for (auto& cmdBuffer : m_CmdBuffers)
	{
		VKSetName<VkCommandBuffer>(m_Device, (uint64_t)cmdBuffer, (std::string(m_CI.debugName) + std::to_string(i)).c_str());
		i++;
	}
	
	m_CmdBufferBIs.resize(m_CI.commandBufferCount);
}

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(m_Device, m_CmdPool, static_cast<uint32_t>(m_CmdBuffers.size()), m_CmdBuffers.data());
}

void CommandBuffer::Begin(uint32_t index, UsageBit usage)
{
	CHECK_VALID_INDEX_RETURN(index);
	
	m_CmdBufferBIs[index].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	m_CmdBufferBIs[index].pNext = nullptr;
	m_CmdBufferBIs[index].flags = static_cast<VkCommandBufferUsageFlags>(usage);
	m_CmdBufferBIs[index].pInheritanceInfo = nullptr;

	MIRU_ASSERT(vkBeginCommandBuffer(m_CmdBuffers[index], &m_CmdBufferBIs[index]), "ERROR: VULKAN: Failed to begin CommandBuffer.");
}

void CommandBuffer::End(uint32_t index)
{
	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(vkEndCommandBuffer(m_CmdBuffers[index]), "ERROR: VULKAN: Failed to end CommandBuffer.");
}

void CommandBuffer::Reset(uint32_t index, bool releaseResources)
{
	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(vkResetCommandBuffer(m_CmdBuffers[index], releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0), "ERROR: VULKAN: Failed to reset CommandBuffer.");
}

void CommandBuffer::ExecuteSecondaryCommandBuffers(uint32_t index, Ref<crossplatform::CommandBuffer> commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices)
{
	if (commandBuffer->GetCreateInfo().level == Level::PRIMARY)
		return;

	CHECK_VALID_INDEX_RETURN(index);
	
	std::vector<VkCommandBuffer>secondaryCmdBuffers;
	for (auto& secondaryIndex : secondaryCommandBufferIndices)
	{
		if (secondaryIndex < commandBuffer->GetCreateInfo().commandBufferCount)
			secondaryCmdBuffers.push_back(std::dynamic_pointer_cast<CommandBuffer>(commandBuffer)->m_CmdBuffers[secondaryIndex]);
	}
	
	vkCmdExecuteCommands(m_CmdBuffers[index], static_cast<uint32_t>(secondaryCmdBuffers.size()), secondaryCmdBuffers.data());
}

void CommandBuffer::Submit(const std::vector<uint32_t>& cmdBufferIndices, std::vector<Ref<crossplatform::Semaphore>>& waits, std::vector<Ref<crossplatform::Semaphore>>& signals, crossplatform::PipelineStageBit pipelineStage, Ref<crossplatform::Fence> fence)
{
	std::vector<VkCommandBuffer>submitCmdBuffers;
	for (auto& index : cmdBufferIndices)
	{
		if (index < m_CI.commandBufferCount)
			submitCmdBuffers.push_back(m_CmdBuffers[index]);
	}
	std::vector<VkSemaphore> vkWaits;
	for (auto& wait : waits)
		vkWaits.push_back(std::dynamic_pointer_cast<Semaphore>(wait)->m_Semaphore);

	std::vector<VkSemaphore> vkSignals;
	for (auto& signal : signals)
		vkSignals.push_back(std::dynamic_pointer_cast<Semaphore>(signal)->m_Semaphore);

	Ref<Context> context = std::dynamic_pointer_cast<Context>(m_CI.pCommandPool->GetCreateInfo().pContext);
	VkQueue queue = context->m_Queues[m_CI.pCommandPool->GetCreateInfo().queueFamilyIndex][0];

	m_CmdBufferSI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_CmdBufferSI.pNext = nullptr;
	m_CmdBufferSI.waitSemaphoreCount = static_cast<uint32_t>(vkWaits.size());
	m_CmdBufferSI.pWaitSemaphores = vkWaits.data();
	m_CmdBufferSI.pWaitDstStageMask = reinterpret_cast<VkPipelineStageFlags*>(&pipelineStage);
	m_CmdBufferSI.commandBufferCount = static_cast<uint32_t>(submitCmdBuffers.size());
	m_CmdBufferSI.pCommandBuffers = submitCmdBuffers.data();
	m_CmdBufferSI.signalSemaphoreCount = static_cast<uint32_t>(vkSignals.size());
	m_CmdBufferSI.pSignalSemaphores = vkSignals.data();

	MIRU_ASSERT(vkQueueSubmit(queue, 1, &m_CmdBufferSI, std::dynamic_pointer_cast<Fence>(fence)->m_Fence), "ERROR: VULKAN: Failed to submit Queue.");
}

void CommandBuffer::Present(const std::vector<uint32_t>& cmdBufferIndices, Ref<crossplatform::Swapchain> swapchain, std::vector<Ref<crossplatform::Fence>>& draws, std::vector<Ref<crossplatform::Semaphore>>& acquires, std::vector<Ref<crossplatform::Semaphore>>& submits)
{
	size_t swapchainImageCount = std::dynamic_pointer_cast<Swapchain>(swapchain)->m_SwapchainImages.size();

	if (swapchainImageCount != cmdBufferIndices.size()
		|| swapchainImageCount != draws.size()
		|| swapchainImageCount != acquires.size()
		|| swapchainImageCount != submits.size())
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: SwapchainImageCount and number of synchronisation objects does not match.");
	}
	Ref<Context> context = std::dynamic_pointer_cast<Context>(m_CI.pCommandPool->GetCreateInfo().pContext);

	VkDevice& vkDevice = context->m_Device;
	VkQueue& vkQueue = context->m_Queues[m_CI.pCommandPool->GetCreateInfo().queueFamilyIndex][0];
	VkSwapchainKHR& vkSwapchain = std::dynamic_pointer_cast<Swapchain>(swapchain)->m_Swapchain;

	draws[m_CurrentFrame]->Wait();
	draws[m_CurrentFrame]->Reset();

	uint32_t imageIndex;
	MIRU_ASSERT(vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX, std::dynamic_pointer_cast<Semaphore>(acquires[m_CurrentFrame])->m_Semaphore, VK_NULL_HANDLE, &imageIndex), "ERROR: VULKAN: Failed to acquire next Image from Swapchain.");

	std::vector<Ref<crossplatform::Semaphore>> _acquires = { acquires[m_CurrentFrame] };
	std::vector<Ref<crossplatform::Semaphore>> _submits = { submits[m_CurrentFrame] };
	Submit({ cmdBufferIndices[m_CurrentFrame] }, _acquires, _submits, crossplatform::PipelineStageBit::COLOR_ATTACHMENT_OUTPUT_BIT, draws[m_CurrentFrame]);

	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &std::dynamic_pointer_cast<Semaphore>(submits[m_CurrentFrame])->m_Semaphore;
	pi.swapchainCount = 1;
	pi.pSwapchains = &vkSwapchain;
	pi.pImageIndices = &imageIndex;
	pi.pResults = nullptr;

	MIRU_ASSERT(vkQueuePresentKHR(vkQueue, &pi), "ERROR: VULKAN: Failed to present the Image from Swapchain.");

	m_CurrentFrame = ((m_CurrentFrame + (size_t)1) % swapchainImageCount);
}

void CommandBuffer::SetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage)
{
	CHECK_VALID_INDEX_RETURN(index);
	vkCmdSetEvent(m_CmdBuffers[index], std::dynamic_pointer_cast<Event>(event)->m_Event, static_cast<VkPipelineStageFlags>(pipelineStage));
}

void CommandBuffer::ResetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage)
{
	CHECK_VALID_INDEX_RETURN(index);
	vkCmdResetEvent(m_CmdBuffers[index], std::dynamic_pointer_cast<Event>(event)->m_Event, static_cast<VkPipelineStageFlags>(pipelineStage));
}

void CommandBuffer::WaitEvents(uint32_t index, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkEvent>vkEvents;
	vkEvents.reserve(events.size());
	for (auto& event : events)
		vkEvents.push_back(std::dynamic_pointer_cast<Event>(event)->m_Event);

	std::vector<VkMemoryBarrier>vkMemoryBarriers;
	std::vector<VkBufferMemoryBarrier>vkBufferBarriers;
	std::vector<VkImageMemoryBarrier>vkImageBarriers;
	for (auto& barrier : barriers)
	{
		Ref<Barrier> _barrier = std::dynamic_pointer_cast<Barrier>(barrier);
		switch (_barrier->GetCreateInfo().type)
		{
		case Barrier::Type::MEMORY:
			vkMemoryBarriers.push_back(_barrier->m_MB); continue;
		case Barrier::Type::BUFFER:
			vkBufferBarriers.push_back(_barrier->m_BMB); continue;
		case Barrier::Type::IMAGE:
			vkImageBarriers.push_back(_barrier->m_IMB); continue;
		default:
			continue;
		}
	}

	vkCmdWaitEvents(m_CmdBuffers[index],
		static_cast<uint32_t>(vkEvents.size()), vkEvents.data(),
		static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(srcStage),
		static_cast<uint32_t>(vkMemoryBarriers.size()), vkMemoryBarriers.data(),
		static_cast<uint32_t>(vkBufferBarriers.size()), vkBufferBarriers.data(),
		static_cast<uint32_t>(vkImageBarriers.size()), vkImageBarriers.data());

}

void CommandBuffer::PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkMemoryBarrier>vkMemoryBarriers;
	std::vector<VkBufferMemoryBarrier>vkBufferBarriers;
	std::vector<VkImageMemoryBarrier>vkImageBarriers;
	for (auto& barrier : barriers)
	{
		Ref<Barrier> _barrier = std::dynamic_pointer_cast<Barrier>(barrier);
		switch (_barrier->GetCreateInfo().type)
		{
		case Barrier::Type::MEMORY:
			vkMemoryBarriers.push_back(_barrier->m_MB); continue;
		case Barrier::Type::BUFFER:
			vkBufferBarriers.push_back(_barrier->m_BMB); continue;
		case Barrier::Type::IMAGE:
			vkImageBarriers.push_back(_barrier->m_IMB); continue;
		default:
			continue;
		}
	}

	vkCmdPipelineBarrier(m_CmdBuffers[index],
		static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkDependencyFlags>(0), 
		static_cast<uint32_t>(vkMemoryBarriers.size()), vkMemoryBarriers.data(),
		static_cast<uint32_t>(vkBufferBarriers.size()), vkBufferBarriers.data(),
		static_cast<uint32_t>(vkImageBarriers.size()), vkImageBarriers.data());
}

void CommandBuffer::ClearColourImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkImageSubresourceRange> vkSubResources;
	vkSubResources.reserve(subresourceRanges.size());
	for (auto& subresourceRange : subresourceRanges)
		vkSubResources.push_back({
		static_cast<VkImageAspectFlags>(subresourceRange.aspect),
		subresourceRange.baseMipLevel,
		subresourceRange.mipLevelCount,
		subresourceRange.baseArrayLayer,
		subresourceRange.arrayLayerCount });

	VkClearColorValue* vkClearColour = (VkClearColorValue*)&clear;

	vkCmdClearColorImage(m_CmdBuffers[index], std::dynamic_pointer_cast<Image>(image)->m_Image, static_cast<VkImageLayout>(layout), vkClearColour, static_cast<uint32_t>(vkSubResources.size()), vkSubResources.data());
}

void CommandBuffer::ClearDepthStencilImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkImageSubresourceRange> vkSubResources;
	vkSubResources.reserve(subresourceRanges.size());
	for (auto& subresourceRange : subresourceRanges)
		vkSubResources.push_back({
		static_cast<VkImageAspectFlags>(subresourceRange.aspect),
		subresourceRange.baseMipLevel,
		subresourceRange.mipLevelCount,
		subresourceRange.baseArrayLayer,
		subresourceRange.arrayLayerCount });

	VkClearDepthStencilValue* vkClearDepthStencil = (VkClearDepthStencilValue*)&clear;

	vkCmdClearDepthStencilImage(m_CmdBuffers[index], std::dynamic_pointer_cast<Image>(image)->m_Image, static_cast<VkImageLayout>(layout), vkClearDepthStencil, static_cast<uint32_t>(vkSubResources.size()), vkSubResources.data());
}