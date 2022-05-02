#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKCommandPoolBuffer.h"

#include "VKContext.h"
#include "VKSwapchain.h"
#include "VKSync.h"
#include "VKImage.h"
#include "VKBuffer.h"
#include "VKPipeline.h"
#include "VKFramebuffer.h"
#include "VKDescriptorPoolSet.h"
#include "VKAccelerationStructure.h"

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
	m_CmdPoolCI.queueFamilyIndex = GetQueueFamilyIndex(m_CI.queueType);

	MIRU_ASSERT(vkCreateCommandPool(m_Device, &m_CmdPoolCI, nullptr, &m_CmdPool), "ERROR: VULKAN: Failed to create CommandPool.");
	VKSetName<VkCommandPool>(m_Device, m_CmdPool, m_CI.debugName);
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

uint32_t CommandPool::GetQueueFamilyIndex(const CommandPool::QueueType& type)
{
	uint32_t index = 0;
	for (auto& queueFamilyProperty : ref_cast<Context>(m_CI.pContext)->m_QueueFamilyProperties)
	{
		VkQueueFlagBits flags = static_cast<VkQueueFlagBits>(queueFamilyProperty.queueFlags);
		if (arc::BitwiseCheck(flags, VK_QUEUE_GRAPHICS_BIT)
			&& type == QueueType::GRAPHICS)
		{
			return index;
		}
		if (arc::BitwiseCheck(flags, VK_QUEUE_COMPUTE_BIT)
			&& !arc::BitwiseCheck(flags, VK_QUEUE_GRAPHICS_BIT)
			&& type == QueueType::COMPUTE)
		{
			return index;
		}
		if (arc::BitwiseCheck(flags, VK_QUEUE_TRANSFER_BIT)s
			&& !arc::BitwiseCheck(flags, VK_QUEUE_COMPUTE_BIT)
			&& !arc::BitwiseCheck(flags, VK_QUEUE_GRAPHICS_BIT)
			&& type == QueueType::TRANSFER)
		{
			return index;
		}
		index++;
	}
	return 0; //Default Queue Family Index
};

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
		VKSetName<VkCommandBuffer>(m_Device, cmdBuffer, m_CI.debugName + ": " + std::to_string(i));
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

void CommandBuffer::ExecuteSecondaryCommandBuffers(uint32_t index, const Ref<crossplatform::CommandBuffer>& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices)
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

void CommandBuffer::Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<Ref<crossplatform::Semaphore>>& waits, const std::vector<crossplatform::PipelineStageBit>& waitDstPipelineStages, const std::vector<Ref<crossplatform::Semaphore>>& signals, const Ref<crossplatform::Fence>& fence)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (waits.size() != waitDstPipelineStages.size())
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: The count of Wait Semaphores and Wait Destination PipelineStages does not match.");
	}

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

	const Ref<Context>& context = ref_cast<Context>(m_CI.pCommandPool->GetCreateInfo().pContext);
	const Ref<CommandPool>& pool = ref_cast<CommandPool>(m_CI.pCommandPool);
	VkQueue queue = context->m_Queues[pool->GetQueueFamilyIndex(pool->GetCreateInfo().queueType)][0];

	m_CmdBufferSI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_CmdBufferSI.pNext = nullptr;
	m_CmdBufferSI.waitSemaphoreCount = static_cast<uint32_t>(vkWaits.size());
	m_CmdBufferSI.pWaitSemaphores = vkWaits.data();
	m_CmdBufferSI.pWaitDstStageMask = (VkPipelineStageFlags*)(waitDstPipelineStages.data());
	m_CmdBufferSI.commandBufferCount = static_cast<uint32_t>(submitCmdBuffers.size());
	m_CmdBufferSI.pCommandBuffers = submitCmdBuffers.data();
	m_CmdBufferSI.signalSemaphoreCount = static_cast<uint32_t>(vkSignals.size());
	m_CmdBufferSI.pSignalSemaphores = vkSignals.data();

	MIRU_ASSERT(vkQueueSubmit(queue, 1, &m_CmdBufferSI, vkFence), "ERROR: VULKAN: Failed to submit Queue.");
}

void CommandBuffer::SetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdSetEvent(m_CmdBuffers[index], ref_cast<Event>(event)->m_Event, static_cast<VkPipelineStageFlags>(pipelineStage));
}

void CommandBuffer::ResetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage)
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
		const Ref<Barrier>& _barrier = ref_cast<Barrier>(barrier);
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

void CommandBuffer::PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, crossplatform::DependencyBit dependencies, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkMemoryBarrier>vkMemoryBarriers;
	std::vector<VkBufferMemoryBarrier>vkBufferBarriers;
	std::vector<VkImageMemoryBarrier>vkImageBarriers;
	for (auto& barrier : barriers)
	{
		const Ref<Barrier>& _barrier = ref_cast<Barrier>(barrier);
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
		static_cast<VkPipelineStageFlags>(srcStage), static_cast<VkPipelineStageFlags>(dstStage), static_cast<VkDependencyFlags>(dependencies),
		static_cast<uint32_t>(vkMemoryBarriers.size()), vkMemoryBarriers.data(),
		static_cast<uint32_t>(vkBufferBarriers.size()), vkBufferBarriers.data(),
		static_cast<uint32_t>(vkImageBarriers.size()), vkImageBarriers.data());
}

void CommandBuffer::ClearColourImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
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

void CommandBuffer::ClearDepthStencilImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
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

void CommandBuffer::BeginRenderPass(uint32_t index, const Ref<crossplatform::Framebuffer>& framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues)
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

void CommandBuffer::BindPipeline(uint32_t index, const Ref<crossplatform::Pipeline>& pipeline)
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

void CommandBuffer::BeginRendering(uint32_t index, const crossplatform::RenderingInfo& renderingInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	auto RenderingAttachmentInfo_To_VkRenderingAttachmentInfoKHR = [](const crossplatform::RenderingAttachmentInfo& renderingAttachment) -> VkRenderingAttachmentInfoKHR
	{
		VkRenderingAttachmentInfoKHR vkRenderingAttachment;
		vkRenderingAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		vkRenderingAttachment.pNext = nullptr;
		vkRenderingAttachment.imageView = ref_cast<ImageView>(renderingAttachment.imageView)->m_ImageView;
		vkRenderingAttachment.imageLayout = static_cast<VkImageLayout>(renderingAttachment.imageLayout);
		vkRenderingAttachment.resolveMode = static_cast<VkResolveModeFlagBits>(renderingAttachment.resolveMode);
		vkRenderingAttachment.resolveImageView = renderingAttachment.resolveImageView ? ref_cast<ImageView>(renderingAttachment.resolveImageView)->m_ImageView : VK_NULL_HANDLE;
		vkRenderingAttachment.resolveImageLayout = static_cast<VkImageLayout>(renderingAttachment.resolveImageLayout);
		vkRenderingAttachment.loadOp = static_cast<VkAttachmentLoadOp>(renderingAttachment.loadOp);
		vkRenderingAttachment.storeOp = static_cast<VkAttachmentStoreOp>(renderingAttachment.storeOp);
		vkRenderingAttachment.clearValue = *reinterpret_cast<const VkClearValue*>(&renderingAttachment.clearValue);

		return vkRenderingAttachment;
	};

	std::vector<VkRenderingAttachmentInfoKHR> vkColourAttachments;
	vkColourAttachments.reserve(renderingInfo.colourAttachments.size());
	for (const auto& colourAttachment : renderingInfo.colourAttachments)
		vkColourAttachments.push_back(RenderingAttachmentInfo_To_VkRenderingAttachmentInfoKHR(colourAttachment));

	VkRenderingAttachmentInfoKHR vkDepthAttachment;
	if (renderingInfo.pDepthAttachment)
		vkDepthAttachment = RenderingAttachmentInfo_To_VkRenderingAttachmentInfoKHR(*renderingInfo.pDepthAttachment);

	VkRenderingAttachmentInfoKHR vkStencilAttachment;
	if (renderingInfo.pStencilAttachment)
		vkStencilAttachment = RenderingAttachmentInfo_To_VkRenderingAttachmentInfoKHR(*renderingInfo.pStencilAttachment);

	VkRenderingInfoKHR vkRenderingInfo;
	vkRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
	vkRenderingInfo.pNext = nullptr;
	vkRenderingInfo.flags = static_cast<VkRenderingFlagsKHR>(renderingInfo.flags);
	vkRenderingInfo.renderArea.offset.x = renderingInfo.renderArea.offset.x;
	vkRenderingInfo.renderArea.offset.y = renderingInfo.renderArea.offset.y;
	vkRenderingInfo.renderArea.extent.width = renderingInfo.renderArea.extent.width;
	vkRenderingInfo.renderArea.extent.height = renderingInfo.renderArea.extent.height;
	vkRenderingInfo.layerCount = renderingInfo.layerCount;
	vkRenderingInfo.viewMask = renderingInfo.viewMask;
	vkRenderingInfo.colorAttachmentCount = static_cast<uint32_t>(vkColourAttachments.size());
	vkRenderingInfo.pColorAttachments = vkColourAttachments.data();
	vkRenderingInfo.pDepthAttachment = renderingInfo.pDepthAttachment ? &vkDepthAttachment : nullptr;
	vkRenderingInfo.pStencilAttachment = renderingInfo.pStencilAttachment ? &vkStencilAttachment : nullptr;

	vkCmdBeginRenderingKHR(m_CmdBuffers[index], &vkRenderingInfo);
}

void CommandBuffer::EndRendering(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdEndRenderingKHR(m_CmdBuffers[index]);
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

void CommandBuffer::BindIndexBuffer(uint32_t index, const Ref<crossplatform::BufferView>& indexBufferView)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	VkBuffer& buffer = ref_cast<Buffer>(ref_cast<BufferView>(indexBufferView)->GetCreateInfo().pBuffer)->m_Buffer;
	const BufferView::CreateInfo& ci = indexBufferView->GetCreateInfo();

	VkIndexType type = VK_INDEX_TYPE_UINT16;
	if (ci.stride == 2)
		type = VK_INDEX_TYPE_UINT16;
	else if (ci.stride == 4)
		type = VK_INDEX_TYPE_UINT32;
	else
		MIRU_ASSERT(true, "ERROR: VULKAN: Unknown index type.");

	vkCmdBindIndexBuffer(m_CmdBuffers[index], buffer, static_cast<VkDeviceSize>(ci.offset), type);
}

void CommandBuffer::BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, uint32_t firstSet, const Ref<crossplatform::Pipeline>& pipeline)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkDescriptorSet> vkDescriptorSets;
	for (auto& descriptorSet : descriptorSets)
	{
		for (auto& vkDescriptorSet : ref_cast<const DescriptorSet>(descriptorSet)->m_DescriptorSets)
		{		
			vkDescriptorSets.push_back(vkDescriptorSet);
		}
	}

	vkCmdBindDescriptorSets(m_CmdBuffers[index], static_cast<VkPipelineBindPoint>(pipeline->GetCreateInfo().type), 
		ref_cast<Pipeline>(pipeline)->m_PipelineLayout, firstSet, static_cast<uint32_t>(vkDescriptorSets.size()), vkDescriptorSets.data(), 0, nullptr);
}

void CommandBuffer::DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdDrawIndexed(m_CmdBuffers[index], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdDraw(m_CmdBuffers[index], vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	vkCmdDispatch(m_CmdBuffers[index], groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::BuildAccelerationStructure(uint32_t index, const std::vector<Ref<crossplatform::AccelerationStructureBuildInfo>>& buildGeometryInfos, const std::vector<std::vector<crossplatform::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	//BLAS and TLAS must be built in seperate commands.
	std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vkBuildGeometryInfosBLAS;
	std::vector<std::vector<VkAccelerationStructureBuildRangeInfoKHR>> vkBuildRangeInfosBLAS;
	std::vector<VkAccelerationStructureBuildGeometryInfoKHR> vkBuildGeometryInfosTLAS;
	std::vector<std::vector<VkAccelerationStructureBuildRangeInfoKHR>> vkBuildRangeInfosTLAS;

	for (size_t i = 0; i < buildGeometryInfos.size(); i++)
	{
		VkAccelerationStructureBuildGeometryInfoKHR asbgi = ref_cast<AccelerationStructureBuildInfo>(buildGeometryInfos[i])->m_ASBGI;
		if (asbgi.geometryCount != buildRangeInfos[i].size())
		{
			MIRU_ASSERT(true, "ERROR: VULKAN: Size mismatch between VkAccelerationStructureBuildGeometryInfoKHR::geometryCount and buildRangeInfos[i].size().");
		}
		
		if (asbgi.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR)
		{
			vkBuildGeometryInfosBLAS.push_back(asbgi);
			vkBuildRangeInfosBLAS.push_back({});
			for (auto& bri : buildRangeInfos[i])
				vkBuildRangeInfosBLAS.back().push_back({ bri.primitiveCount, bri.primitiveOffset, bri.firstVertex, bri.transformOffset });

		}
		else if (asbgi.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR)
		{
			vkBuildGeometryInfosTLAS.push_back(asbgi);
			vkBuildRangeInfosTLAS.push_back({});
			for (auto& bri : buildRangeInfos[i])
				vkBuildRangeInfosTLAS.back().push_back({ bri.primitiveCount, bri.primitiveOffset, bri.firstVertex, bri.transformOffset });
		}
		else
		{
			MIRU_ASSERT(true, "ERROR: VULKAN: VkAccelerationStructureTypeKHR is not VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR or VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR.");
		}
	}

	if (!vkBuildGeometryInfosBLAS.empty())
	{
		VkMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(m_CmdBuffers[index], VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		vkCmdBuildAccelerationStructuresKHR(m_CmdBuffers[index], static_cast<uint32_t>(vkBuildGeometryInfosBLAS.size()), vkBuildGeometryInfosBLAS.data(), reinterpret_cast<const VkAccelerationStructureBuildRangeInfoKHR* const*>(vkBuildRangeInfosBLAS.data()));
	}
	if (!vkBuildGeometryInfosTLAS.empty())
	{
		VkMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(m_CmdBuffers[index], VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		vkCmdBuildAccelerationStructuresKHR(m_CmdBuffers[index], static_cast<uint32_t>(vkBuildGeometryInfosTLAS.size()), vkBuildGeometryInfosTLAS.data(), reinterpret_cast<const VkAccelerationStructureBuildRangeInfoKHR* const*>(vkBuildRangeInfosTLAS.data()));
	}
}

void CommandBuffer::TraceRays(uint32_t index, const crossplatform::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pMissShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pHitShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	VkStridedDeviceAddressRegionKHR emptySbtEntry;
	emptySbtEntry.deviceAddress = 0;
	emptySbtEntry.stride = 0;
	emptySbtEntry.size = 0;

	const VkStridedDeviceAddressRegionKHR* raygenSBT = pRaygenShaderBindingTable ? reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(pRaygenShaderBindingTable) : &emptySbtEntry;
	const VkStridedDeviceAddressRegionKHR* missSBT = pMissShaderBindingTable ? reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(pMissShaderBindingTable) : &emptySbtEntry;
	const VkStridedDeviceAddressRegionKHR* hitSBT = pHitShaderBindingTable ? reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(pHitShaderBindingTable) : &emptySbtEntry;
	const VkStridedDeviceAddressRegionKHR* callableSBT = pCallableShaderBindingTable ? reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(pCallableShaderBindingTable) : &emptySbtEntry;
	vkCmdTraceRaysKHR(m_CmdBuffers[index], raygenSBT, missSBT, hitSBT, callableSBT, width, height, depth);
}

void CommandBuffer::CopyBuffer(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Buffer>& dstBuffer, const std::vector<Buffer::Copy>& copyRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<VkBufferCopy> vkBufferCopy;
	vkBufferCopy.reserve(copyRegions.size());
	for (auto& copyRegion : copyRegions)
		vkBufferCopy.push_back({ copyRegion.srcOffset, copyRegion.dstOffset, copyRegion.size});

	vkCmdCopyBuffer(m_CmdBuffers[index], ref_cast<Buffer>(srcBuffer)->m_Buffer, ref_cast<Buffer>(dstBuffer)->m_Buffer, static_cast<uint32_t>(vkBufferCopy.size()), vkBufferCopy.data());
}

void CommandBuffer::CopyImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<Image::Copy>& copyRegions)
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
	
	vkCmdCopyImage(m_CmdBuffers[index], ref_cast<Image>(srcImage)->m_Image, static_cast<VkImageLayout>(srcImageLayout),
		ref_cast<Image>(dstImage)->m_Image, static_cast<VkImageLayout>(dstImageLayout), static_cast<uint32_t>(vkImageCopy.size()), vkImageCopy.data());
}

void CommandBuffer::CopyBufferToImage(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions)
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

void CommandBuffer::CopyImageToBuffer(uint32_t index, const Ref<crossplatform::Image>& srcImage, const Ref<crossplatform::Buffer>& dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions)
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

void CommandBuffer::ResolveImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Resolve>& resolveRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkImageResolve> vkImageResolve;
	vkImageResolve.reserve(resolveRegions.size());
	for (auto& resolveRegion : resolveRegions)
	{
		VkImageResolve ir;
		ir.srcSubresource = { static_cast<VkImageAspectFlags>(resolveRegion.srcSubresource.aspectMask), resolveRegion.srcSubresource.mipLevel, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
		ir.srcOffset = { resolveRegion.srcOffset.x, resolveRegion.srcOffset.y, resolveRegion.srcOffset.z };
		ir.dstSubresource = { static_cast<VkImageAspectFlags>(resolveRegion.dstSubresource.aspectMask), resolveRegion.dstSubresource.mipLevel, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
		ir.dstOffset = { resolveRegion.dstOffset.x, resolveRegion.dstOffset.y, resolveRegion.dstOffset.z };
		ir.extent = { resolveRegion.extent.width, resolveRegion.extent.height, resolveRegion.extent.depth };
		vkImageResolve.push_back(ir);
	}

	vkCmdResolveImage(m_CmdBuffers[index], ref_cast<Image>(srcImage)->m_Image, static_cast<VkImageLayout>(srcImageLayout), 
		ref_cast<Image>(dstImage)->m_Image, static_cast<VkImageLayout>(dstImageLayout), static_cast<uint32_t>(vkImageResolve.size()), vkImageResolve.data());
}

void CommandBuffer::BeginDebugLabel(uint32_t index, const std::string& label, std::array<float, 4> rgba)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	VkDebugUtilsLabelEXT vkLabel;
	vkLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	vkLabel.pNext = nullptr;
	vkLabel.pLabelName = label.c_str();
	vkLabel.color[0] = rgba[0];
	vkLabel.color[1] = rgba[1];
	vkLabel.color[2] = rgba[2];
	vkLabel.color[3] = rgba[3];
	if (vkCmdBeginDebugUtilsLabelEXT)
		vkCmdBeginDebugUtilsLabelEXT(m_CmdBuffers[index], &vkLabel);
}

void CommandBuffer::EndDebugLabel(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	if (vkCmdEndDebugUtilsLabelEXT)
		vkCmdEndDebugUtilsLabelEXT(m_CmdBuffers[index]);
}

void CommandBuffer::SetViewport(uint32_t index, const std::vector<crossplatform::Viewport>& viewports)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkViewport> vkViewports;
	vkViewports.reserve(viewports.size());
	for (auto& viewport : viewports)
		vkViewports.push_back({ viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth });

	vkCmdSetViewport(m_CmdBuffers[index], 0, static_cast<uint32_t>(vkViewports.size()), vkViewports.data());
}

void CommandBuffer::SetScissor(uint32_t index, const std::vector<crossplatform::Rect2D>& scissors)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<VkRect2D> vkRect2D;
	vkRect2D.reserve(scissors.size());
	for (auto& scissor : scissors)
		vkRect2D.push_back({ {scissor.offset.x, scissor.offset.y}, {scissor.extent.width, scissor.extent.height} });

	vkCmdSetScissor(m_CmdBuffers[index], 0, static_cast<uint32_t>(vkRect2D.size()), vkRect2D.data());
}
#endif