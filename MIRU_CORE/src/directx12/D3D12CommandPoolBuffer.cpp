#include "common.h"
#include "D3D12CommandPoolBuffer.h"
#include "D3D12Context.h"
#include "D3D12Swapchain.h"
#include "D3D12Sync.h"
#include "D3D12Buffer.h"
#include "D3D12Image.h"
#include "D3D12Pipeline.h"
#include "D3D12DescriptorPoolSet.h"
#include "D3D12Framebuffer.h"

using namespace miru;
using namespace d3d12;

//CmdPool
CommandPool::CommandPool(CommandPool::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Context>(pCreateInfo->pContext)->m_Device),
	m_Queue(ref_cast<Context>(pCreateInfo->pContext)->m_Queues[pCreateInfo->queueFamilyIndex])
{
	m_CI = *pCreateInfo;

	MIRU_ASSERT(m_Device->CreateCommandAllocator(m_Queue->GetDesc().Type, IID_PPV_ARGS(&m_CmdPool)), "ERROR: D3D12: Failed to create CommandPool.");
	D3D12SetName(m_CmdPool, m_CI.debugName);
}

CommandPool::~CommandPool()
{
	SAFE_RELEASE(m_CmdPool);
}

void CommandPool::Trim()
{
	return;
}

void CommandPool::Reset(bool releaseResources)
{
	m_CmdPool->Reset();
}

//CmdBuffer
CommandBuffer::CommandBuffer(CommandBuffer::CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_Device = ref_cast<CommandPool>(m_CI.pCommandPool)->m_Device;
	m_CmdPool = ref_cast<CommandPool>(m_CI.pCommandPool)->m_CmdPool;
	D3D12_COMMAND_QUEUE_DESC queueDesc = ref_cast<CommandPool>(m_CI.pCommandPool)->m_Queue->GetDesc();

	m_CmdBuffers.resize(m_CI.commandBufferCount);
	for (size_t i = 0; i < m_CmdBuffers.size(); i++)
	{
		MIRU_ASSERT(m_Device->CreateCommandList(0, m_CI.level == Level::SECONDARY ? D3D12_COMMAND_LIST_TYPE_BUNDLE : queueDesc.Type, m_CmdPool, nullptr, IID_PPV_ARGS(&m_CmdBuffers[i])), "ERROR: D3D12: Failed to create CommandBuffer.");
		D3D12SetName(m_CmdBuffers[i], (std::string(m_CI.debugName) + ": " + std::to_string(i)).c_str());
		End(static_cast<uint32_t>(i));
	}

}

CommandBuffer::~CommandBuffer()
{
	for (auto& cmdBuffer : m_CmdBuffers)
		SAFE_RELEASE(cmdBuffer);
}

void CommandBuffer::Begin(uint32_t index, UsageBit usage)
{
	Reset(index, false);
}

void CommandBuffer::End(uint32_t index)
{
	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Close(), "ERROR: D3D12: Failed to end CommandBuffer.");
}

void CommandBuffer::Reset(uint32_t index, bool releaseResources)
{
	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Reset(m_CmdPool, nullptr), "ERROR: D3D12: Failed to reset CommandBuffer.");
}

void CommandBuffer::ExecuteSecondaryCommandBuffers(uint32_t index, Ref<crossplatform::CommandBuffer> commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices)
{
	if (commandBuffer->GetCreateInfo().level == Level::PRIMARY)
		return;

	CHECK_VALID_INDEX_RETURN(index);
	if (m_CmdBuffers[index]->GetType() != D3D12_COMMAND_LIST_TYPE_DIRECT)
		return;

	for (auto& secondaryIndex : secondaryCommandBufferIndices)
	{
		if (secondaryIndex < commandBuffer->GetCreateInfo().commandBufferCount)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ExecuteBundle(reinterpret_cast<ID3D12GraphicsCommandList*>(ref_cast<CommandBuffer>(commandBuffer)->m_CmdBuffers[secondaryIndex]));
	}

}

void CommandBuffer::Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<Ref<crossplatform::Semaphore>>& waits, const std::vector<Ref<crossplatform::Semaphore>>& signals, crossplatform::PipelineStageBit pipelineStage, Ref<crossplatform::Fence> fence)
{
	ID3D12CommandQueue* queue = ref_cast<CommandPool>(m_CI.pCommandPool)->m_Queue;
	std::vector<ID3D12CommandList*>submitCmdBuffers;
	for (auto& index : cmdBufferIndices)
	{
		if (index < m_CI.commandBufferCount)
			submitCmdBuffers.push_back(m_CmdBuffers[index]);
	}
	for (auto& wait : waits)
		queue->Wait(ref_cast<Semaphore>(wait)->m_Semaphore, ref_cast<Semaphore>(wait)->GetValue());
	
	queue->ExecuteCommandLists(static_cast<uint32_t>(submitCmdBuffers.size()), submitCmdBuffers.data());
	
	for (auto& signal : signals)
	{
		ref_cast<Semaphore>(signal)->GetValue()++;
		queue->Signal(ref_cast<Semaphore>(signal)->m_Semaphore, ref_cast<Semaphore>(signal)->GetValue());
	}
}

void CommandBuffer::Present(const std::vector<uint32_t>& cmdBufferIndices, Ref<crossplatform::Swapchain> swapchain, const std::vector<Ref<crossplatform::Fence>>& draws, const std::vector<Ref<crossplatform::Semaphore>>& acquires, const std::vector<Ref<crossplatform::Semaphore>>& submits, bool& windowResize)
{
	size_t swapchainImageCount = ref_cast<Swapchain>(swapchain)->m_SwapchainRTVs.size();

	if (swapchainImageCount != cmdBufferIndices.size()
		|| swapchainImageCount != draws.size()
		|| swapchainImageCount != acquires.size()
		|| swapchainImageCount != submits.size())
	{
		MIRU_ASSERT(true, "ERROR: D3D12: SwapchainImageCount and number of synchronisation objects does not match.");
	}
	
	IDXGISwapChain4* d3d12Swapchain = ref_cast<Swapchain>(swapchain)->m_Swapchain;
	ID3D12CommandQueue* d3d12Queue = ref_cast<CommandPool>(m_CI.pCommandPool)->m_Queue;

	draws[m_CurrentFrame]->Wait();
	draws[m_CurrentFrame]->Reset();

	//UINT imageIndex = d3d12Swapchain->GetCurrentBackBufferIndex();
	Submit({ cmdBufferIndices[m_CurrentFrame] }, {}, {}, crossplatform::PipelineStageBit::NONE, {});
	
	MIRU_ASSERT(d3d12Swapchain->Present(1, 0), "ERROR: D3D12: Failed to present the Image from Swapchain.");
	ref_cast<Fence>(draws[m_CurrentFrame])->GetValue()++;
	d3d12Queue->Signal(ref_cast<Fence>(draws[m_CurrentFrame])->m_Fence, ref_cast<Fence>(draws[m_CurrentFrame])->GetValue());

	m_CurrentFrame = ((m_CurrentFrame + (size_t)1) % swapchainImageCount);
}

void CommandBuffer::SetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage)
{
}

void CommandBuffer::ResetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage)
{
}

void CommandBuffer::WaitEvents(uint32_t index, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
}

void CommandBuffer::PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	CHECK_VALID_INDEX_RETURN(index);

	std::vector<D3D12_RESOURCE_BARRIER> _barriers;
	for (auto& barrier : barriers)
	{
		for (auto& _barrier : ref_cast<Barrier>(barrier)->m_Barriers)
			_barriers.push_back(_barrier);

	}
	if (_barriers.empty())
		return;

	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ResourceBarrier(static_cast<UINT>(_barriers.size()), _barriers.data());
}

void CommandBuffer::ClearColourImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	UINT descriptorCount = 0;
	for (size_t h = 0; h < subresourceRanges.size(); h++)
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].mipLevelCount; i++)
			descriptorCount++;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = descriptorCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ID3D12DescriptorHeap* heap;
	MIRU_ASSERT(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)), "ERROR: D3D12: Failed to create temporary DescriptorHeap for RenderTargetViews.");
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC resourceDesc = ref_cast<Image>(image)->m_ResourceDesc;
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (size_t h = 0; h < subresourceRanges.size(); h++)
	{
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].mipLevelCount; i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC m_RTVDesc = {};
			m_RTVDesc.Format = resourceDesc.Format;
			switch (resourceDesc.Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_UNKNOWN:
			{
				m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			}
			case D3D12_RESOURCE_DIMENSION_BUFFER:
			{
				m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
				m_RTVDesc.Buffer.FirstElement = 0;
				m_RTVDesc.Buffer.NumElements = static_cast<UINT>(resourceDesc.Width);
				break;
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
					m_RTVDesc.Texture1DArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
					m_RTVDesc.Texture1DArray.ArraySize = subresourceRanges[h].arrayLayerCount;
					m_RTVDesc.Texture1DArray.MipSlice = i;
					break;
				}
				else
				{
					m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
					m_RTVDesc.Texture1D.MipSlice = i;
					break;
				}
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
						m_RTVDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						m_RTVDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						break;
					}
					else
					{
						m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						m_RTVDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						m_RTVDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						m_RTVDesc.Texture2D.MipSlice = i;
						m_RTVDesc.Texture2D.PlaneSlice = 0;
						break;
					}
				}
				else
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
						break;
					}
					else
					{
						m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						m_RTVDesc.Texture2D.MipSlice = i;
						m_RTVDesc.Texture2D.PlaneSlice = 0;
						break;
					}
				}
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			{
				m_RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
				m_RTVDesc.Texture3D.FirstWSlice = subresourceRanges[h].baseArrayLayer;
				m_RTVDesc.Texture3D.WSize = subresourceRanges[h].arrayLayerCount;
				m_RTVDesc.Texture3D.MipSlice = i;
				break;
			}
			}
			m_Device->CreateRenderTargetView(ref_cast<Image>(image)->m_Image, 0/*&m_RTVDesc*/, handle);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(handle, clear.float32, 0, nullptr);
			handle.ptr += rtvDescriptorSize;
		}
	}

	handle = heap->GetCPUDescriptorHandleForHeapStart();
	SAFE_RELEASE(heap);
}
void CommandBuffer::ClearDepthStencilImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	UINT descriptorCount = 0;
	for (size_t h = 0; h < subresourceRanges.size(); h++)
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].mipLevelCount; i++)
			descriptorCount++;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = descriptorCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ID3D12DescriptorHeap* heap;
	MIRU_ASSERT(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)), "ERROR: D3D12: Failed to create temporary DescriptorHeap for DepthStencilViews.");
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC resourceDesc = ref_cast<Image>(image)->m_ResourceDesc;
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	for (size_t h = 0; h < subresourceRanges.size(); h++)
	{
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].mipLevelCount; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC m_DSVDesc;
			m_DSVDesc.Format = resourceDesc.Format;
			switch (resourceDesc.Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_BUFFER:
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			case D3D12_RESOURCE_DIMENSION_UNKNOWN:
			{
				m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
					m_DSVDesc.Texture1DArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
					m_DSVDesc.Texture1DArray.ArraySize = subresourceRanges[h].arrayLayerCount;
					m_DSVDesc.Texture1DArray.MipSlice = i;
					break;
				}
				else
				{
					m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
					m_DSVDesc.Texture1D.MipSlice = i;
					break;
				}
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
						m_DSVDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						m_DSVDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						break;
					}
					else
					{
						m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
						m_DSVDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						m_DSVDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						m_DSVDesc.Texture2D.MipSlice = i;
						break;
					}
				}
				else
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
						break;
					}
					else
					{
						m_DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
						m_DSVDesc.Texture2D.MipSlice = i;
						break;
					}
				}
			}
			}
			m_Device->CreateDepthStencilView(ref_cast<Image>(image)->m_Image, 0/*&m_DSVDesc*/, handle);
			handle.ptr += rtvDescriptorSize;
			descriptorCount++;
		}
	}

	handle = heap->GetCPUDescriptorHandleForHeapStart();
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear.depth, shrink_uint32_t_to_uint8_t(clear.stencil), 0, nullptr);
	SAFE_RELEASE(heap);
}

void CommandBuffer::BeginRenderPass(uint32_t index, Ref<crossplatform::Framebuffer> framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues) 
{
	CHECK_VALID_INDEX_RETURN(index);
	m_RenderPassFramebuffer = framebuffer;
	m_RenderPassClearValues = clearValues;
	
	//Transition resources to be begin render pass.
	m_SubpassIndex = (uint32_t)-1;
	Ref<crossplatform::RenderPass> renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;

	std::vector<Ref<crossplatform::Barrier>> barriers;
	crossplatform::Barrier::CreateInfo barrierCI = {};
	barrierCI.type = crossplatform::Barrier::Type::IMAGE;
	barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;

	size_t i = 0;
	for (auto& imageView : m_RenderPassFramebuffer->GetCreateInfo().attachments)
	{
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = renderPass->GetCreateInfo().attachments[i].initialLayout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		i++;
	}
	PipelineBarrier(index, crossplatform::PipelineStageBit::BOTTOM_OF_PIPE_BIT, crossplatform::PipelineStageBit::TOP_OF_PIPE_BIT, barriers);

	//Begin first subpass
	NextSubpass(index);
};

void CommandBuffer::EndRenderPass(uint32_t index) 
{
	CHECK_VALID_INDEX_RETURN(index);

	//Transition resources to be end render pass.
	Ref<crossplatform::RenderPass> renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;
	m_SubpassIndex = (uint32_t)-1;

	std::vector<Ref<crossplatform::Barrier>> barriers;
	crossplatform::Barrier::CreateInfo barrierCI = {};
	barrierCI.type = crossplatform::Barrier::Type::IMAGE;
	barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;

	size_t i = 0;
	for (auto& imageView : m_RenderPassFramebuffer->GetCreateInfo().attachments)
	{
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = renderPass->GetCreateInfo().attachments[i].finalLayout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		i++;
	}
	PipelineBarrier(index, crossplatform::PipelineStageBit::BOTTOM_OF_PIPE_BIT, crossplatform::PipelineStageBit::TOP_OF_PIPE_BIT, barriers);
};

void CommandBuffer::NextSubpass(uint32_t index)
{
	CHECK_VALID_INDEX_RETURN(index);
	m_SubpassIndex++;
	Ref<crossplatform::RenderPass> renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;
	RenderPass::SubpassDescription subpassDesc = renderPass->GetCreateInfo().subpassDescriptions[m_SubpassIndex];
	const std::vector<Ref<crossplatform::ImageView>>& framebufferAttachments = m_RenderPassFramebuffer->GetCreateInfo().attachments;
	const std::vector<crossplatform::RenderPass::AttachmentDescription>& renderpassAttachments = renderPass->GetCreateInfo().attachments;

	//Transition resources for the subpass.
	std::vector<Ref<crossplatform::Barrier>> barriers;
	crossplatform::Barrier::CreateInfo barrierCI = {};
	barrierCI.type = crossplatform::Barrier::Type::IMAGE;
	barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	for (auto& input : subpassDesc.inputAttachments)
	{
		Ref<crossplatform::ImageView> imageView = framebufferAttachments[input.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = input.layout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
	}
	for (auto& colour : subpassDesc.colourAttachments)
	{
		Ref<crossplatform::ImageView> imageView = framebufferAttachments[colour.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = colour.layout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
	}
	for (auto& resolve : subpassDesc.resolveAttachments)
	{
		Ref<crossplatform::ImageView> imageView = framebufferAttachments[resolve.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = resolve.layout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
	}
	for (auto& depthStencil : subpassDesc.depthStencilAttachment)
	{
		Ref<crossplatform::ImageView> imageView = framebufferAttachments[depthStencil.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = depthStencil.layout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
	}
	for (auto& preseverse : subpassDesc.preseverseAttachments)
	{
		Ref<crossplatform::ImageView> imageView = framebufferAttachments[preseverse.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.newLayout = preseverse.layout;
		barrierCI.subresoureRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
	}
	PipelineBarrier(index, crossplatform::PipelineStageBit::BOTTOM_OF_PIPE_BIT, crossplatform::PipelineStageBit::TOP_OF_PIPE_BIT, barriers);

	//Set RenderTargets
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
	for (auto& attachment : subpassDesc.colourAttachments)
		rtvs.push_back(ref_cast<ImageView>(framebufferAttachments[attachment.attachmentIndex])->m_RTVDescHandle);
	if(!subpassDesc.depthStencilAttachment.empty())
		dsv = ref_cast<ImageView>(framebufferAttachments[subpassDesc.depthStencilAttachment[0].attachmentIndex])->m_DSVDescHandle;
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), false, dsv.ptr?&dsv:nullptr);

	//Clear imageviews
	uint32_t attachId = 0;
	for (auto& attachment : subpassDesc.colourAttachments)
	{
		attachId = attachment.attachmentIndex;
		if (ref_cast<Framebuffer>(m_RenderPassFramebuffer)->m_ImageView_RTV_DSV_SRVs[attachId].HasRTV && renderpassAttachments[attachId].loadOp > RenderPass::AttachmentLoadOp::LOAD)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(ref_cast<ImageView>(framebufferAttachments[attachId])->m_RTVDescHandle, m_RenderPassClearValues[attachId].colour.float32, 0, nullptr);
	}
	if (!subpassDesc.depthStencilAttachment.empty())
	{
		attachId = subpassDesc.depthStencilAttachment[0].attachmentIndex;
		if (ref_cast<Framebuffer>(m_RenderPassFramebuffer)->m_ImageView_RTV_DSV_SRVs[attachId].HasDSV)
		{
			D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
			if (renderpassAttachments[attachId].loadOp > RenderPass::AttachmentLoadOp::LOAD)
				flags |= D3D12_CLEAR_FLAG_DEPTH;
			if (renderpassAttachments[attachId].stencilLoadOp > RenderPass::AttachmentLoadOp::LOAD)
				flags |= D3D12_CLEAR_FLAG_STENCIL;

			if(flags)
				reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(ref_cast<ImageView>(framebufferAttachments[attachId])->m_DSVDescHandle, flags, m_RenderPassClearValues[attachId].depthStencil.depth, shrink_uint32_t_to_uint8_t(m_RenderPassClearValues[attachId].depthStencil.stencil), 0, nullptr);
		}
	}
}

void CommandBuffer::BindPipeline(uint32_t index, Ref<crossplatform::Pipeline> pipeline) 
{
	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetPipelineState(ref_cast<Pipeline>(pipeline)->m_Pipeline);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetGraphicsRootSignature(ref_cast<Pipeline>(pipeline)->m_RootSignature);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetViewports(static_cast<UINT>(ref_cast<Pipeline>(pipeline)->m_Viewports.size()), ref_cast<Pipeline>(pipeline)->m_Viewports.data());
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetScissorRects(static_cast<UINT>(ref_cast<Pipeline>(pipeline)->m_Scissors.size()), ref_cast<Pipeline>(pipeline)->m_Scissors.data());
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

};

void CommandBuffer::BindVertexBuffers(uint32_t index, const std::vector<Ref<crossplatform::BufferView>>& vertexBufferViews) 
{
	CHECK_VALID_INDEX_RETURN(index);

	std::vector<D3D12_VERTEX_BUFFER_VIEW>vbvs;
	vbvs.reserve(vertexBufferViews.size());
	for (auto& vbv : vertexBufferViews)
		vbvs.push_back(ref_cast<BufferView>(vbv)->m_VBVDesc);

	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetVertexBuffers(0, static_cast<UINT>(vbvs.size()), vbvs.data());

};
void CommandBuffer::BindIndexBuffer(uint32_t index, Ref<crossplatform::BufferView> indexBufferView) 
{
	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetIndexBuffer(&ref_cast<BufferView>(indexBufferView)->m_IBVDesc);
};

bool operator== (const D3D12_ROOT_DESCRIPTOR_TABLE& a, const D3D12_ROOT_DESCRIPTOR_TABLE& b) { return (a.NumDescriptorRanges == b.NumDescriptorRanges) && (a.pDescriptorRanges == b.pDescriptorRanges); }
void CommandBuffer::BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, Ref<crossplatform::Pipeline> pipeline) 
{
	CHECK_VALID_INDEX_RETURN(index);
	std::vector<ID3D12DescriptorHeap*> descHeaps;
	for (auto& descriptorSet : descriptorSets)
	{
		for (auto& descriptorPool : ref_cast<DescriptorPool>(descriptorSet->GetCreateInfo().pDescriptorPool)->m_DescriptorPools)
		{
			for (size_t i = 0; i < 4; i++)
			{
				ID3D12DescriptorHeap* descHeap = descriptorPool[i];
				if (descHeap)
					descHeaps.push_back(descHeap);
			}
		}
	}
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetDescriptorHeaps(static_cast<UINT>(descHeaps.size()), descHeaps.data());

	for (size_t i = 0; i < ref_cast<Pipeline>(pipeline)->m_RootParameters.size(); i++)
	{
		D3D12_ROOT_DESCRIPTOR_TABLE pipeline_table = ref_cast<Pipeline>(pipeline)->m_RootParameters[i].DescriptorTable;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle;

		for (auto& descriptorSet : descriptorSets)
		{
			for (auto& rootParam : ref_cast<DescriptorSet>(descriptorSet)->m_RootParameters)
			{
				D3D12_ROOT_DESCRIPTOR_TABLE descSet_table = rootParam.DescriptorTable;
				if (pipeline_table == descSet_table)
				{
					for (size_t i = 0; i < descSet_table.NumDescriptorRanges; i++)
					{
						if (descSet_table.pDescriptorRanges[i].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
							gpuDescHandle = ref_cast<DescriptorPool>(descriptorSet->GetCreateInfo().pDescriptorPool)->m_DescriptorPools[i][1]->GetGPUDescriptorHandleForHeapStart();
						else
							gpuDescHandle = ref_cast<DescriptorPool>(descriptorSet->GetCreateInfo().pDescriptorPool)->m_DescriptorPools[i][0]->GetGPUDescriptorHandleForHeapStart();
					}
				}
			}
		}

		if (pipeline->GetCreateInfo().type ==  crossplatform::PipelineType::GRAPHICS)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuDescHandle);
		else if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::COMPUTE)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootDescriptorTable(static_cast<UINT>(i), gpuDescHandle);

	}
};

void CommandBuffer::DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
};

void CommandBuffer::CopyBuffer(uint32_t index, Ref<crossplatform::Buffer> srcBuffer, Ref<crossplatform::Buffer> dstBuffer, const std::vector<crossplatform::Buffer::Copy>& copyRegions) 
{
	CHECK_VALID_INDEX_RETURN(index);
	for (auto& copyRegion : copyRegions)
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyBufferRegion(
			ref_cast<Buffer>(dstBuffer)->m_Buffer, static_cast<UINT>(copyRegion.dstOffset), 
			ref_cast<Buffer>(srcBuffer)->m_Buffer, static_cast<UINT>(copyRegion.srcOffset), static_cast<UINT>(copyRegion.size));
};

void CommandBuffer::CopyImage(uint32_t index, Ref<crossplatform::Image> srcImage, Ref<crossplatform::Image> dstImage, const std::vector<crossplatform::Image::Copy>& copyRegions) 
{
	CHECK_VALID_INDEX_RETURN(index);
	for (auto& copyRegion : copyRegions)
	{
		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = ref_cast<Image>(dstImage)->m_Image;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = ref_cast<Image>(srcImage)->m_Image;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_BOX srcbox;
		srcbox.left = static_cast<UINT>(copyRegion.srcOffset.x);
		srcbox.top = static_cast<UINT>(copyRegion.srcOffset.y);
		srcbox.front = static_cast<UINT>(copyRegion.srcOffset.z);
		srcbox.right = static_cast<UINT>(copyRegion.extent.width);
		srcbox.bottom = static_cast<UINT>(copyRegion.extent.height);
		srcbox.back = static_cast<UINT>(copyRegion.extent.depth);

		for (uint32_t i = copyRegion.dstSubresource.baseArrayLayer; i < copyRegion.dstSubresource.arrayLayerCount; i++)
		{
			const D3D12_RESOURCE_DESC& dstResDesc = dst.pResource->GetDesc();
			const D3D12_RESOURCE_DESC& srcResDesc = src.pResource->GetDesc();
			dst.SubresourceIndex = Image::D3D12CalculateSubresource(copyRegion.dstSubresource.mipLevel, i, 0, dstResDesc.MipLevels, dstResDesc.DepthOrArraySize);
			src.SubresourceIndex = Image::D3D12CalculateSubresource(copyRegion.srcSubresource.mipLevel, i, 0, srcResDesc.MipLevels, srcResDesc.DepthOrArraySize);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, copyRegion.dstOffset.x, copyRegion.dstOffset.y, copyRegion.dstOffset.z, &src, &srcbox);
		}
	}
};

void CommandBuffer::CopyBufferToImage(uint32_t index, Ref<crossplatform::Buffer> srcBuffer, Ref<crossplatform::Image> dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy> regions)
{
	CHECK_VALID_INDEX_RETURN(index);
	for (auto& region : regions)
	{
		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = ref_cast<Image>(dstImage)->m_Image;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = ref_cast<Buffer>(srcBuffer)->m_Buffer;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layout;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		UINT64 RequiredSize;
		m_Device->GetCopyableFootprints(&dst.pResource->GetDesc(), 0, 1, 0, &Layout, &NumRows, &RowSizesInBytes, &RequiredSize);
		src.PlacedFootprint = Layout;

		for (uint32_t i = region.imageSubresource.baseArrayLayer; i < region.imageSubresource.arrayLayerCount; i++)
		{
			const D3D12_RESOURCE_DESC& dstResDesc = dst.pResource->GetDesc();
			dst.SubresourceIndex = Image::D3D12CalculateSubresource(region.imageSubresource.mipLevel, i, 0, dstResDesc.MipLevels, dstResDesc.DepthOrArraySize);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z, &src, nullptr);
		}
	}	
}

void CommandBuffer::CopyImageToBuffer(uint32_t index, Ref<crossplatform::Image> srcImage, Ref<crossplatform::Buffer> dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy> regions)
{
	CHECK_VALID_INDEX_RETURN(index);
	for (auto& region : regions)
	{
		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = ref_cast<Buffer>(dstBuffer)->m_Buffer;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = ref_cast<Image>(srcImage)->m_Image;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layout;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		UINT64 RequiredSize;
		m_Device->GetCopyableFootprints(&src.pResource->GetDesc(), 0, 1, 0, &Layout, &NumRows, &RowSizesInBytes, &RequiredSize);
		dst.PlacedFootprint = Layout;

		for (uint32_t i = region.imageSubresource.baseArrayLayer; i < region.imageSubresource.arrayLayerCount; i++)
		{
			const D3D12_RESOURCE_DESC& srcResDesc = src.pResource->GetDesc();
			src.SubresourceIndex = Image::D3D12CalculateSubresource(region.imageSubresource.mipLevel, i, 0, srcResDesc.MipLevels, srcResDesc.DepthOrArraySize);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z, &src, nullptr);
		}
	}
}