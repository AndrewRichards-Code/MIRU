#include "common.h"
#include "VKCommandPoolBuffer.h"

#include "VKContext.h"
#include "VKSwapchain.h"
#include "VKSync.h"
#include "VKImage.h"
#include "VKBuffer.h"
#include "VKPipeline.h"
#include "VKFramebuffer.h"
#include "VKDescriptorPoolSet.h"

using namespace miru;
using namespace vulkan;

//CmdPool
CommandPool::CommandPool(CommandPool::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->pContext->GetDevice()))
{
	MIRU_CPU_PROFILE_FUNCTION();

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
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyCommandPool(m_Device, m_CmdPool, nullptr);
}

void CommandPool::Trim()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkTrimCommandPool(m_Device, m_CmdPool, 0);
}

void CommandPool::Reset(bool releaseResources)
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_ASSERT(vkResetCommandPool(m_Device, m_CmdPool, releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0), "ERROR: VULKAN: Failed to reset CommandPool.");
}

//CmdBuffer
CommandBuffer::CommandBuffer(CommandBuffer::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<CommandPool>(pCreateInfo->pCommandPool)->m_Device),
	m_CmdPool(ref_cast<CommandPool>(pCreateInfo->pCommandPool)->m_CmdPool)
{
	MIRU_CPU_PROFILE_FUNCTION();

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
	MIRU_CPU_PROFILE_FUNCTION();

	vkFreeCommandBuffers(m_Device, m_CmdPool, static_cast<uint32_t>(m_CmdBuffers.size()), m_CmdBuffers.data());
}

void CommandBuffer::Begin(uint32_t index, UsageBit usage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	
	m_CmdBufferBIs[index].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	m_CmdBufferBIs[index].pNext = nullptr;
	m_CmdBufferBIs[index].flags = static_cast<VkCommandBufferUsageFlags>(usage);
	m_CmdBufferBIs[index].pInheritanceInfo = nullptr;

	MIRU_ASSERT(vkBeginCommandBuffer(m_CmdBuffers[index], &m_CmdBufferBIs[index]), "ERROR: VULKAN: Failed to begin CommandBuffer.");
}

void CommandBuffer::End(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(vkEndCommandBuffer(m_CmdBuffers[index]), "ERROR: VULKAN: Failed to end CommandBuffer.");
}

void CommandBuffer::Reset(uint32_t index, bool releaseResources)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(vkResetCommandBuffer(m_CmdBuffers[index], releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0), "ERROR: VULKAN: Failed to reset CommandBuffer.");
}

void CommandBuffer::ExecuteSecondaryCommandBuffers(uint32_t index, Ref<crossplatform::CommandBuffer> commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (commandBuffer->GetCreateInfo().level == Level::PRIMARY)
		return;

	CHECK_VALID_INDEX_RETURN(index);
	
	std::vector<VkCommandBuffer>secondaryCmdBuffers;
	for (auto& secondaryIndex : secondaryCommandBufferIndices)
	{
		if (secondaryIndex < commandBuffer->GetCreateInfo().commandBufferCount)
			secondaryCmdBuffers.push_back(ref_cast<CommandBuffer>(commandBuffer)->m_CmdBuffers[secondaryIndex]);
	}
	
	vkCmdExecuteCommands(m_CmdBuffers[index], static_cast<uint32_t>(secondaryCmdBuffers.size()), secondaryCmdBuffers.data());
}

void CommandBuffer::Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<Ref<crossplatform::Semaphore>>& waits, const std::vector<Ref<crossplatform::Semaphore>>& signals, crossplatform::PipelineStageBit pipelineStage, Ref<crossplatform::Fence> fence)
{
	MIRU_CPU_PROFILE_FUNCTION();

	std::vector<VkCommandBuffer>submitCmdBuffers;
	for (auto& index : cmdBufferIndices)
	{
		if (index < m_CI.commandBufferCount)
			submitCmdBuffers.push_back(m_CmdBuffers[index]);
	}
	std::vector<VkSemaphore> vkWaits;
	for (auto& wait : waits)
		vkWaits.push_back(ref_cast<Semaphore>(wait)->m_Semaphore);

	std::vector<VkSemaphore> vkSignals;
	for (auto& signal : signals)
		vkSignals.push_back(ref_cast<Semaphore>(signal)->m_Semaphore);

	VkFence vkFence = fence ? ref_cast<Fence>(fence)->m_Fence : VK_NULL_HANDLE;

	Ref<Context> context = ref_cast<Context>(m_CI.pCommandPool->GetCreateInfo().pContext);
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

	MIRU_ASSERT(vkQueueSubmit(queue, 1, &m_CmdBufferSI, vkFence), "ERROR: VULKAN: Failed to submit Queue.");
}

void CommandBuffer::Present(const std::vector<uint32_t>& cmdBufferIndices, Ref<crossplatform::Swapchain> swapchain, const std::vector<Ref<crossplatform::Fence>>& draws, const std::vector<Ref<crossplatform::Semaphore>>& acquires, const std::vector<Ref<crossplatform::Semaphore>>& submits, bool& windowResize)
{
	MIRU_CPU_PROFILE_FUNCTION();

	size_t swapchainImageCount = ref_cast<Swapchain>(swapchain)->m_SwapchainImages.size();

	if (swapchainImageCount != cmdBufferIndices.size()
		|| swapchainImageCount != draws.size()
		|| swapchainImageCount != acquires.size()
		|| swapchainImageCount != submits.size())
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: SwapchainImageCount and number of synchronisation objects does not match.");
	}
	Ref<Context> context = ref_cast<Context>(m_CI.pCommandPool->GetCreateInfo().pContext);

	VkDevice& vkDevice = context->m_Device;
	VkQueue& vkQueue = context->m_Queues[m_CI.pCommandPool->GetCreateInfo().queueFamilyIndex][0];
	VkSwapchainKHR& vkSwapchain = ref_cast<Swapchain>(swapchain)->m_Swapchain;

	draws[m_CurrentFrame]->Wait();
	draws[m_CurrentFrame]->Reset();

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX, ref_cast<Semaphore>(acquires[m_CurrentFrame])->m_Semaphore, VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowResize)
	{
		windowResize = false;
		swapchain->m_Resized = true;
		vkDeviceWaitIdle(*reinterpret_cast<VkDevice*>(swapchain->GetCreateInfo().pContext->GetDevice()));
		return;
	}
	else
		MIRU_ASSERT(result, "ERROR: VULKAN: Failed to acquire next Image from Swapchain.");

	std::vector<Ref<crossplatform::Semaphore>> _acquires = { acquires[m_CurrentFrame] };
	std::vector<Ref<crossplatform::Semaphore>> _submits = { submits[m_CurrentFrame] };
	Submit({ cmdBufferIndices[m_CurrentFrame] }, _acquires, _submits, crossplatform::PipelineStageBit::COLOR_ATTACHMENT_OUTPUT_BIT, draws[m_CurrentFrame]);

	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &ref_cast<Semaphore>(submits[m_CurrentFrame])->m_Semaphore;
	pi.swapchainCount = 1;
	pi.pSwapchains = &vkSwapchain;
	pi.pImageIndices = &imageIndex;
	pi.pResults = nullptr;

	result = vkQueuePresentKHR(vkQueue, &pi);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowResize)
	{
		windowResize = false;
		swapchain->m_Resized = true;
		vkDeviceWaitIdle(*reinterpret_cast<VkDevice*>(swapchain->GetCreateInfo().pContext->GetDevice()));
		return;
	}
	else
		MIRU_ASSERT(result, "ERROR: VULKAN: Failed to present the Image from Swapchain.");

	m_CurrentFrame = ((m_CurrentFrame + (size_t)1) % swapchainImageCount);
}

void CommandBuffer::SetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdSetEvent(m_CmdBuffers[index], ref_cast<Event>(event)->m_Event, static_cast<VkPipelineStageFlags>(pipelineStage));
}

void CommandBuffer::ResetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdResetEvent(m_CmdBuffers[index], ref_cast<Event>(event)->m_Event, static_cast<VkPipelineStageFlags>(pipelineStage));
}

void CommandBuffer::WaitEvents(uint32_t index, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkEvent>vkEvents;
	vkEvents.reserve(events.size());
	for (auto& event : events)
		vkEvents.push_back(ref_cast<Event>(event)->m_Event);

	std::vector<VkMemoryBarrier>vkMemoryBarriers;
	std::vector<VkBufferMemoryBarrier>vkBufferBarriers;
	std::vector<VkImageMemoryBarrier>vkImageBarriers;
	for (auto& barrier : barriers)
	{
		Ref<Barrier> _barrier = ref_cast<Barrier>(barrier);
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
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkMemoryBarrier>vkMemoryBarriers;
	std::vector<VkBufferMemoryBarrier>vkBufferBarriers;
	std::vector<VkImageMemoryBarrier>vkImageBarriers;
	for (auto& barrier : barriers)
	{
		Ref<Barrier> _barrier = ref_cast<Barrier>(barrier);
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
		static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(dstStage), static_cast<VkDependencyFlags>(0),
		static_cast<uint32_t>(vkMemoryBarriers.size()), vkMemoryBarriers.data(),
		static_cast<uint32_t>(vkBufferBarriers.size()), vkBufferBarriers.data(),
		static_cast<uint32_t>(vkImageBarriers.size()), vkImageBarriers.data());
}

void CommandBuffer::ClearColourImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	MIRU_CPU_PROFILE_FUNCTION();

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

	vkCmdClearColorImage(m_CmdBuffers[index], ref_cast<Image>(image)->m_Image, static_cast<VkImageLayout>(layout), vkClearColour, static_cast<uint32_t>(vkSubResources.size()), vkSubResources.data());
}

void CommandBuffer::ClearDepthStencilImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	MIRU_CPU_PROFILE_FUNCTION();

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

	vkCmdClearDepthStencilImage(m_CmdBuffers[index], ref_cast<Image>(image)->m_Image, static_cast<VkImageLayout>(layout), vkClearDepthStencil, static_cast<uint32_t>(vkSubResources.size()), vkSubResources.data());
}

void CommandBuffer::BeginRenderPass(uint32_t index, Ref<crossplatform::Framebuffer> framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkClearValue> vkClearValue;
	vkClearValue.reserve(clearValues.size());
	for (auto& clearValue : clearValues)
		vkClearValue.push_back(*reinterpret_cast<const VkClearValue*>(&clearValue));

	VkRenderPassBeginInfo bi;
	bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	bi.pNext = nullptr;
	bi.renderPass = ref_cast<RenderPass>(framebuffer->GetCreateInfo().renderPass)->m_RenderPass;
	bi.framebuffer = ref_cast<Framebuffer>(framebuffer)->m_Framebuffer;
	bi.renderArea.offset = { 0,0 };
	bi.renderArea.extent.width = framebuffer->GetCreateInfo().width;
	bi.renderArea.extent.height= framebuffer->GetCreateInfo().height;
	bi.clearValueCount = static_cast<uint32_t>(vkClearValue.size());
	bi.pClearValues = vkClearValue.data();

	vkCmdBeginRenderPass(m_CmdBuffers[index], &bi, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdEndRenderPass(m_CmdBuffers[index]);
}

void CommandBuffer::BindPipeline(uint32_t index, Ref<crossplatform::Pipeline> pipeline)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdBindPipeline(m_CmdBuffers[index], static_cast<VkPipelineBindPoint>(pipeline->GetCreateInfo().type), ref_cast<Pipeline>(pipeline)->m_Pipeline);
}

void CommandBuffer::NextSubpass(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdNextSubpass(m_CmdBuffers[index], VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::BindVertexBuffers(uint32_t index, const std::vector<Ref<crossplatform::BufferView>>& vertexBufferViews)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkBuffer> vkBuffers;
	std::vector<VkDeviceSize> offsets;
	for (auto& vetexBufferView : vertexBufferViews)
	{
		vkBuffers.push_back(ref_cast<Buffer>(ref_cast<BufferView>(vetexBufferView)->GetCreateInfo().pBuffer)->m_Buffer);
		offsets.push_back(ref_cast<BufferView>(vetexBufferView)->GetCreateInfo().offset);
	}

	vkCmdBindVertexBuffers(m_CmdBuffers[index], 0, static_cast<uint32_t>(vkBuffers.size()), vkBuffers.data(), offsets.data());
}

void CommandBuffer::BindIndexBuffer(uint32_t index, Ref<crossplatform::BufferView> indexBufferView)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	VkBuffer& buffer = ref_cast<Buffer>(ref_cast<BufferView>(indexBufferView)->GetCreateInfo().pBuffer)->m_Buffer;
	const BufferView::CreateInfo& ci = indexBufferView->GetCreateInfo();

	VkIndexType type;
	if (ci.stride == 2)
		type = VK_INDEX_TYPE_UINT16;
	else if (ci.stride == 4)
		type = VK_INDEX_TYPE_UINT32;
	else
		MIRU_ASSERT(true, "ERROR: VULKAN: Unknown index type.");

	vkCmdBindIndexBuffer(m_CmdBuffers[index], buffer, static_cast<VkDeviceSize>(ci.offset), type);
}

void CommandBuffer::BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, Ref<crossplatform::Pipeline> pipeline)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkDescriptorSet> vkDescriptorSets;
	uint32_t descriptorSetCount = 0;
	for (auto& descriptorSet : descriptorSets)
	{
		for (auto& vkDescriptorSet : ref_cast<const DescriptorSet>(descriptorSet)->m_DescriptorSets)
		{		
			vkDescriptorSets.push_back(vkDescriptorSet);
			descriptorSetCount++;
		}
	}

	vkCmdBindDescriptorSets(m_CmdBuffers[index], static_cast<VkPipelineBindPoint>(pipeline->GetCreateInfo().type),
		ref_cast<Pipeline>(pipeline)->m_PipelineLayout, 0, descriptorSetCount, vkDescriptorSets.data(), 0, nullptr);
}

void CommandBuffer::DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdDrawIndexed(m_CmdBuffers[index], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::CopyBuffer(uint32_t index, Ref<crossplatform::Buffer> srcBuffer, Ref<crossplatform::Buffer> dstBuffer, const std::vector<Buffer::Copy>& copyRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkBufferCopy> vkBufferCopy;
	vkBufferCopy.reserve(copyRegions.size());
	for (auto& copyRegion : copyRegions)
		vkBufferCopy.push_back({ copyRegion.srcOffset, copyRegion.dstOffset, copyRegion.size});

	vkCmdCopyBuffer(m_CmdBuffers[index], ref_cast<Buffer>(srcBuffer)->m_Buffer, ref_cast<Buffer>(dstBuffer)->m_Buffer, static_cast<uint32_t>(vkBufferCopy.size()), vkBufferCopy.data());
}

void CommandBuffer::CopyImage(uint32_t index, Ref<crossplatform::Image> srcImage, Ref<crossplatform::Image> dstImage, const std::vector<Image::Copy>& copyRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkImageCopy> vkImageCopy;
	vkImageCopy.reserve(copyRegions.size());
	for (auto& copyRegion : copyRegions)
	{
		VkImageCopy ic;
		ic.srcSubresource = { static_cast<VkImageAspectFlags>(copyRegion.srcSubresource.aspectMask), copyRegion.srcSubresource.mipLevel, copyRegion.srcSubresource.baseArrayLayer, copyRegion.srcSubresource.arrayLayerCount };
		ic.srcOffset = { copyRegion.srcOffset.x, copyRegion.srcOffset.y, copyRegion.srcOffset.z };
		ic.dstSubresource = { static_cast<VkImageAspectFlags>(copyRegion.dstSubresource.aspectMask), copyRegion.dstSubresource.mipLevel, copyRegion.dstSubresource.baseArrayLayer, copyRegion.dstSubresource.arrayLayerCount };
		ic.dstOffset = { copyRegion.dstOffset.x, copyRegion.dstOffset.y, copyRegion.dstOffset.z };
		ic.extent = { copyRegion.extent.width, copyRegion.extent.height, copyRegion.extent.depth};
		vkImageCopy.push_back(ic);
	}
	
	vkCmdCopyImage(m_CmdBuffers[index], ref_cast<Image>(srcImage)->m_Image, ref_cast<Image>(srcImage)->m_CurrentLayout,
		ref_cast<Image>(dstImage)->m_Image, ref_cast<Image>(dstImage)->m_CurrentLayout, static_cast<uint32_t>(vkImageCopy.size()), vkImageCopy.data());
}

void miru::vulkan::CommandBuffer::CopyBufferToImage(uint32_t index, Ref<crossplatform::Buffer> srcBuffer, Ref<crossplatform::Image> dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy> regions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkBufferImageCopy> vkBufferImageCopy;
	for (auto& region : regions)
	{
		VkBufferImageCopy bic;
		bic.bufferOffset = static_cast<VkDeviceSize>(region.bufferOffset);
		bic.bufferRowLength = region.bufferRowLength;
		bic.bufferImageHeight = region.bufferImageHeight;
		bic.imageSubresource = { static_cast<VkImageAspectFlags>(region.imageSubresource.aspectMask), region.imageSubresource.mipLevel, region.imageSubresource.baseArrayLayer, region.imageSubresource.arrayLayerCount };
		bic.imageOffset = { region.imageOffset.x, region.imageOffset.y, region.imageOffset.z };
		bic.imageExtent = { region.imageExtent.width, region.imageExtent.height, region.imageExtent.depth };
		vkBufferImageCopy.push_back(bic);
	}

	vkCmdCopyBufferToImage(m_CmdBuffers[index], ref_cast<Buffer>(srcBuffer)->m_Buffer, ref_cast<Image>(dstImage)->m_Image, static_cast<VkImageLayout>(dstImageLayout), static_cast<uint32_t>(vkBufferImageCopy.size()), vkBufferImageCopy.data());
}

void miru::vulkan::CommandBuffer::CopyImageToBuffer(uint32_t index, Ref<crossplatform::Image> srcImage, Ref<crossplatform::Buffer> dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy> regions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkBufferImageCopy> vkBufferImageCopy;
	for (auto& region : regions)
	{
		VkBufferImageCopy bic;
		bic.bufferOffset = static_cast<VkDeviceSize>(region.bufferOffset);
		bic.bufferRowLength = region.bufferRowLength;
		bic.bufferImageHeight = region.bufferImageHeight;
		bic.imageSubresource = { static_cast<VkImageAspectFlags>(region.imageSubresource.aspectMask), region.imageSubresource.mipLevel, region.imageSubresource.baseArrayLayer, region.imageSubresource.arrayLayerCount };
		bic.imageOffset = { region.imageOffset.x, region.imageOffset.y, region.imageOffset.z };
		bic.imageExtent = { region.imageExtent.width, region.imageExtent.height, region.imageExtent.depth };
		vkBufferImageCopy.push_back(bic);
	}

	vkCmdCopyImageToBuffer(m_CmdBuffers[index], ref_cast<Image>(srcImage)->m_Image, static_cast<VkImageLayout>(srcImageLayout), ref_cast<Buffer>(dstBuffer)->m_Buffer, static_cast<uint32_t>(vkBufferImageCopy.size()), vkBufferImageCopy.data());
}
\