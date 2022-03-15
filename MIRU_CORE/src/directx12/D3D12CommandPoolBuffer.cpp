#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12CommandPoolBuffer.h"
#include "D3D12Context.h"
#include "D3D12Swapchain.h"
#include "D3D12Sync.h"
#include "D3D12Buffer.h"
#include "D3D12Image.h"
#include "D3D12Pipeline.h"
#include "D3D12DescriptorPoolSet.h"
#include "D3D12Framebuffer.h"
#include "D3D12AccelerationStructure.h"

using namespace miru;
using namespace d3d12;

//CmdPool
CommandPool::CommandPool(CommandPool::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Context>(pCreateInfo->pContext)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Queue = ref_cast<Context>(pCreateInfo->pContext)->m_Queues[GetCommandQueueIndex(pCreateInfo->queueType)];
}

CommandPool::~CommandPool()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto& cmdPool : m_CmdPools)
	{
		MIRU_D3D12_SAFE_RELEASE(cmdPool);
	}
}

void CommandPool::Trim()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandPool::Reset(bool releaseResources)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if ((m_CI.flags & FlagBit::RESET_COMMAND_BUFFER_BIT) == FlagBit::RESET_COMMAND_BUFFER_BIT)
	{
		for (auto& cmdPool : m_CmdPools)
		{
			MIRU_ASSERT(cmdPool->Reset(), "ERROR: D3D12: Failed to reset CommandPool.");
		}
	}
}

uint32_t CommandPool::GetCommandQueueIndex(const CommandPool::QueueType& type)
{
	uint32_t index = 0;
	for (auto& queueDesc : ref_cast<Context>(m_CI.pContext)->m_QueueDescs)
	{
		D3D12_COMMAND_LIST_TYPE flags = queueDesc.Type;
		if (flags == D3D12_COMMAND_LIST_TYPE_DIRECT && type == QueueType::GRAPHICS)
			return index;
		if (flags == D3D12_COMMAND_LIST_TYPE_COMPUTE && type == QueueType::COMPUTE)
			return index;
		if (flags == D3D12_COMMAND_LIST_TYPE_COPY && type == QueueType::TRANSFER)
			return index;

		index++;
	}
	return 0; //Default Command Queue Index
};

//CmdBuffer
CommandBuffer::CommandBuffer(CommandBuffer::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	Ref<CommandPool> cmdPool = ref_cast<CommandPool>(m_CI.pCommandPool);
	m_Device = cmdPool->m_Device;
	D3D12_COMMAND_QUEUE_DESC queueDesc = cmdPool->m_Queue->GetDesc();
	std::vector<ID3D12CommandAllocator*>& d3d12CmdAllocators = cmdPool->m_CmdPools;

	d3d12CmdAllocators.resize(m_CI.commandBufferCount);
	m_CmdBuffers.resize(m_CI.commandBufferCount);
	for (size_t i = 0; i < m_CmdBuffers.size(); i++)
	{
		MIRU_ASSERT(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&d3d12CmdAllocators[i])), "ERROR: D3D12: Failed to create CommandPool.");
		D3D12SetName(d3d12CmdAllocators[i], cmdPool->GetCreateInfo().debugName + ": " + std::to_string(i));
	
		MIRU_ASSERT(m_Device->CreateCommandList(0, m_CI.level == Level::SECONDARY ? D3D12_COMMAND_LIST_TYPE_BUNDLE : queueDesc.Type, d3d12CmdAllocators[i], nullptr, IID_PPV_ARGS(&m_CmdBuffers[i])), "ERROR: D3D12: Failed to create CommandBuffer.");
		D3D12SetName(m_CmdBuffers[i], m_CI.debugName + ": " + std::to_string(i));
		End(static_cast<uint32_t>(i));
	}

	switch (ref_cast<Context>(m_CI.pCommandPool->GetCreateInfo().pContext)->m_Features.d3d12Options.ResourceBindingTier)
	{
	case D3D12_RESOURCE_BINDING_TIER_3:
	{
		m_MaxDescriptorCount 
			= m_MaxCBVsPerStage 
			= m_MaxSRVsPerStage 
			= m_MaxUAVsPerStage 
			= m_MaxSamplersPerStage 
			= 1000000;
		break;
	}
	case D3D12_RESOURCE_BINDING_TIER_2:
	{
		m_MaxDescriptorCount = 1000000;
		m_MaxCBVsPerStage = 14;
		m_MaxSRVsPerStage = 1000000;
		m_MaxUAVsPerStage = 64;
		m_MaxSamplersPerStage = 1000000;
		break;
	}
	case D3D12_RESOURCE_BINDING_TIER_1:
	default:
	{
		const auto& contextRI = m_CI.pCommandPool->GetCreateInfo().pContext->GetResultInfo();
		m_MaxDescriptorCount = 1000000;
		m_MaxCBVsPerStage = 14;
		m_MaxSRVsPerStage = 128;
		m_MaxUAVsPerStage = (contextRI.apiVersionMajor == 11 && contextRI.apiVersionMinor == 0) ? 8 : 64;
		m_MaxSamplersPerStage = 16;
		break;
	}
	}

	m_CmdBuffer_CBV_SRV_UAV_DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_CmdBuffer_CBV_SRV_UAV_DescriptorHeapDesc.NumDescriptors = m_MaxDescriptorCount;
	m_CmdBuffer_CBV_SRV_UAV_DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_CmdBuffer_CBV_SRV_UAV_DescriptorHeapDesc.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&m_CmdBuffer_CBV_SRV_UAV_DescriptorHeapDesc, IID_PPV_ARGS(&m_CmdBuffer_CBV_SRV_UAV_DescriptorHeap));

	m_CmdBuffer_Sampler_DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	m_CmdBuffer_Sampler_DescriptorHeapDesc.NumDescriptors = m_MaxSamplerCount;
	m_CmdBuffer_Sampler_DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_CmdBuffer_Sampler_DescriptorHeapDesc.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&m_CmdBuffer_Sampler_DescriptorHeapDesc, IID_PPV_ARGS(&m_CmdBuffer_Sampler_DescriptorHeap));

	m_CmdBuffer_CBV_SRV_UAV_DescriptorOffset = 0;
	m_CmdBuffer_SamplerDescriptorOffset = 0;
}

CommandBuffer::~CommandBuffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_CmdBuffer_CBV_SRV_UAV_DescriptorHeap);
	MIRU_D3D12_SAFE_RELEASE(m_CmdBuffer_Sampler_DescriptorHeap);

	for (auto& cmdBuffer : m_CmdBuffers)
	{
		MIRU_D3D12_SAFE_RELEASE(cmdBuffer);
	}
}

void CommandBuffer::Begin(uint32_t index, UsageBit usage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	Reset(index, false);

	m_SetDescriptorHeaps_PerCmdBuffer.resize(m_CmdBuffers.size());
	for (auto setDescriptorHeap : m_SetDescriptorHeaps_PerCmdBuffer)
		setDescriptorHeap = true;

	if (index == 0)
	{
		m_CmdBuffer_CBV_SRV_UAV_DescriptorOffset = 0;
		m_CmdBuffer_SamplerDescriptorOffset = 0;
	}
}

void CommandBuffer::End(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	MIRU_ASSERT(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Close(), "ERROR: D3D12: Failed to end CommandBuffer.");
	m_Resettable = true;
}

void CommandBuffer::Reset(uint32_t index, bool releaseResources)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_Resettable)
	{
		CHECK_VALID_INDEX_RETURN(index);
		std::vector<ID3D12CommandAllocator*>& d3d12CmdAllocators = ref_cast<CommandPool>(m_CI.pCommandPool)->m_CmdPools;

		MIRU_ASSERT(d3d12CmdAllocators[index]->Reset(), "ERROR: D3D12: Failed to reset CommandPool.");
		MIRU_ASSERT(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Reset(d3d12CmdAllocators[index], nullptr), "ERROR: D3D12: Failed to reset CommandBuffer.");
		m_Resettable = false;
	}
}
void CommandBuffer::ExecuteSecondaryCommandBuffers(uint32_t index, const Ref<crossplatform::CommandBuffer>& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices)
{
	MIRU_CPU_PROFILE_FUNCTION();

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

void CommandBuffer::Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<Ref<crossplatform::Semaphore>>& waits, const std::vector<crossplatform::PipelineStageBit>& waitDstPipelineStages, const std::vector<Ref<crossplatform::Semaphore>>& signals, const Ref<crossplatform::Fence>& fence)
{
	MIRU_CPU_PROFILE_FUNCTION();

	ID3D12CommandQueue* queue = ref_cast<CommandPool>(m_CI.pCommandPool)->m_Queue;
	std::vector<ID3D12CommandList*>submitCmdBuffers;

	for (auto& index : cmdBufferIndices)
	{
		if (index < m_CI.commandBufferCount)
			submitCmdBuffers.push_back(m_CmdBuffers[index]);
	}

	for (auto& wait : waits)
	{
		MIRU_ASSERT(queue->Wait(ref_cast<Semaphore>(wait)->m_Semaphore, ref_cast<Semaphore>(wait)->GetValue()), "ERROR: D3D12: Failed to Wait on the wait Semaphore.");
	}
	
	queue->ExecuteCommandLists(static_cast<uint32_t>(submitCmdBuffers.size()), submitCmdBuffers.data());
	
	for (auto& signal : signals)
	{
		ref_cast<Semaphore>(signal)->GetValue()++;
		MIRU_ASSERT(queue->Signal(ref_cast<Semaphore>(signal)->m_Semaphore, ref_cast<Semaphore>(signal)->GetValue()), "ERROR: D3D12: Failed to Signal the signal Semaphore.");
	}

	if (fence)
	{
		ref_cast<Fence>(fence)->GetValue()++;
		MIRU_ASSERT(queue->Signal(ref_cast<Fence>(fence)->m_Fence, ref_cast<Fence>(fence)->GetValue()), "ERROR: D3D12: Failed to Signal the draw Fence.");
	}
}

void CommandBuffer::SetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandBuffer::ResetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandBuffer::WaitEvents(uint32_t index, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandBuffer::PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, crossplatform::DependencyBit dependencies, const std::vector<Ref<crossplatform::Barrier>>& barriers)
{
	MIRU_CPU_PROFILE_FUNCTION();

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

void CommandBuffer::ClearColourImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	MIRU_CPU_PROFILE_FUNCTION();

	UINT descriptorCount = 0;
	for (size_t h = 0; h < subresourceRanges.size(); h++)
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
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
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
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
	MIRU_D3D12_SAFE_RELEASE(heap);
}

void CommandBuffer::ClearDepthStencilImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges)
{
	MIRU_CPU_PROFILE_FUNCTION();

	UINT descriptorCount = 0;
	for (size_t h = 0; h < subresourceRanges.size(); h++)
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
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
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
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
	MIRU_D3D12_SAFE_RELEASE(heap);
}

void CommandBuffer::BeginRenderPass(uint32_t index, const Ref<crossplatform::Framebuffer>& framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	m_RenderPassFramebuffer = framebuffer;
	m_RenderPassClearValues = clearValues;
	
	//Transition resources to be begin render pass.
	m_SubpassIndex = (uint32_t)-1;
	const Ref<crossplatform::RenderPass>& renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;

	std::vector<Ref<crossplatform::Barrier>> barriers;
	crossplatform::Barrier::CreateInfo barrierCI = {};
	barrierCI.type = crossplatform::Barrier::Type::IMAGE;
	barrierCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
	barrierCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;

	m_RenderPassFramebufferAttachementLayouts.clear();

	size_t i = 0;
	for (auto& imageView : m_RenderPassFramebuffer->GetCreateInfo().attachments)
	{
		if(m_RenderPassFramebuffer->GetCreateInfo().attachments.size() != m_RenderPassFramebufferAttachementLayouts.size())
			m_RenderPassFramebufferAttachementLayouts.push_back(ref_cast<Image>(imageView->GetCreateInfo().pImage)->GetCreateInfo().layout);

		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[i];
		barrierCI.newLayout = renderPass->GetCreateInfo().attachments[i].initialLayout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[i] = barrierCI.newLayout;
		i++;
	}
	PipelineBarrier(index, crossplatform::PipelineStageBit::BOTTOM_OF_PIPE_BIT, crossplatform::PipelineStageBit::TOP_OF_PIPE_BIT, crossplatform::DependencyBit::NONE_BIT, barriers);

	//Begin first subpass
	NextSubpass(index);
};

void CommandBuffer::EndRenderPass(uint32_t index) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	//Resolve any attachments from the previous subpass
	ResolvePreviousSubpassAttachments(index);

	//Transition resources to be end render pass.
	const Ref<crossplatform::RenderPass>& renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;
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
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[i];
		barrierCI.newLayout = renderPass->GetCreateInfo().attachments[i].finalLayout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[i] = barrierCI.newLayout;
		i++;
	}
	PipelineBarrier(index, crossplatform::PipelineStageBit::BOTTOM_OF_PIPE_BIT, crossplatform::PipelineStageBit::TOP_OF_PIPE_BIT, crossplatform::DependencyBit::NONE_BIT, barriers);
};

void CommandBuffer::NextSubpass(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	//Resolve any attachments from the previous subpass
	ResolvePreviousSubpassAttachments(index);

	m_SubpassIndex++;
	const Ref<crossplatform::RenderPass>& renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;
	const RenderPass::SubpassDescription& subpassDesc = renderPass->GetCreateInfo().subpassDescriptions[m_SubpassIndex];
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
		const Ref<crossplatform::ImageView>& imageView = framebufferAttachments[input.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[input.attachmentIndex];
		barrierCI.newLayout = input.layout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[input.attachmentIndex] = barrierCI.newLayout;
	}
	for (auto& colour : subpassDesc.colourAttachments)
	{
		const Ref<crossplatform::ImageView>& imageView = framebufferAttachments[colour.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[colour.attachmentIndex];
		barrierCI.newLayout = colour.layout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[colour.attachmentIndex] = barrierCI.newLayout;
	}
	for (auto& resolve : subpassDesc.resolveAttachments)
	{
		const Ref<crossplatform::ImageView>& imageView = framebufferAttachments[resolve.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[resolve.attachmentIndex];
		barrierCI.newLayout = resolve.layout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[resolve.attachmentIndex] = barrierCI.newLayout;
	}
	for (auto& depthStencil : subpassDesc.depthStencilAttachment)
	{
		const Ref<crossplatform::ImageView>& imageView = framebufferAttachments[depthStencil.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[depthStencil.attachmentIndex];
		barrierCI.newLayout = depthStencil.layout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[depthStencil.attachmentIndex] = barrierCI.newLayout;
	}
	for (auto& preseverse : subpassDesc.preseverseAttachments)
	{
		const Ref<crossplatform::ImageView>& imageView = framebufferAttachments[preseverse.attachmentIndex];
		barrierCI.pImage = imageView->GetCreateInfo().pImage;
		barrierCI.oldLayout = m_RenderPassFramebufferAttachementLayouts[preseverse.attachmentIndex];
		barrierCI.newLayout = preseverse.layout;
		barrierCI.subresourceRange = imageView->GetCreateInfo().subresourceRange;
		barriers.push_back(crossplatform::Barrier::Create(&barrierCI));
		m_RenderPassFramebufferAttachementLayouts[preseverse.attachmentIndex] = barrierCI.newLayout;
	}
	PipelineBarrier(index, crossplatform::PipelineStageBit::BOTTOM_OF_PIPE_BIT, crossplatform::PipelineStageBit::TOP_OF_PIPE_BIT, crossplatform::DependencyBit::NONE_BIT, barriers);

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
		if (ref_cast<Framebuffer>(m_RenderPassFramebuffer)->m_ImageView_RTV_DSV_SRVs[attachId].HasRTV && renderpassAttachments[attachId].loadOp == RenderPass::AttachmentLoadOp::CLEAR)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(ref_cast<ImageView>(framebufferAttachments[attachId])->m_RTVDescHandle, m_RenderPassClearValues[attachId].colour.float32, 0, nullptr);
	}
	if (!subpassDesc.depthStencilAttachment.empty())
	{
		attachId = subpassDesc.depthStencilAttachment[0].attachmentIndex;
		if (ref_cast<Framebuffer>(m_RenderPassFramebuffer)->m_ImageView_RTV_DSV_SRVs[attachId].HasDSV)
		{
			D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
			if (renderpassAttachments[attachId].loadOp == RenderPass::AttachmentLoadOp::CLEAR)
				flags |= D3D12_CLEAR_FLAG_DEPTH;
			if (renderpassAttachments[attachId].stencilLoadOp == RenderPass::AttachmentLoadOp::CLEAR)
				flags |= D3D12_CLEAR_FLAG_STENCIL;

			if(flags)
				reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(ref_cast<ImageView>(framebufferAttachments[attachId])->m_DSVDescHandle, flags, m_RenderPassClearValues[attachId].depthStencil.depth, shrink_uint32_t_to_uint8_t(m_RenderPassClearValues[attachId].depthStencil.stencil), 0, nullptr);
		}
	}
}

void CommandBuffer::BeginRendering(uint32_t index, const crossplatform::RenderingInfo& renderingInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	m_RenderingInfo = renderingInfo;

	//Set RenderTargets
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
	for (auto& attachment : m_RenderingInfo.colourAttachments)
		rtvs.push_back(ref_cast<ImageView>(attachment.imageView)->m_RTVDescHandle);
	if (m_RenderingInfo.pDepthAttachment)
		dsv = ref_cast<ImageView>(m_RenderingInfo.pDepthAttachment->imageView)->m_DSVDescHandle;
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), false, dsv.ptr?&dsv:nullptr);

	//Clear imageviews
	uint32_t attachId = 0;
	for (auto& attachment : m_RenderingInfo.colourAttachments)
	{
		if (attachment.loadOp == RenderPass::AttachmentLoadOp::CLEAR)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(ref_cast<ImageView>(attachment.imageView)->m_RTVDescHandle, attachment.clearValue.colour.float32, 0, nullptr);
	}
	if (m_RenderingInfo.pDepthAttachment && m_RenderingInfo.pStencilAttachment)
	{
		D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
		if (m_RenderingInfo.pDepthAttachment->loadOp == RenderPass::AttachmentLoadOp::CLEAR)
			flags |= D3D12_CLEAR_FLAG_DEPTH;
		if (m_RenderingInfo.pStencilAttachment->loadOp == RenderPass::AttachmentLoadOp::CLEAR)
			flags |= D3D12_CLEAR_FLAG_STENCIL;

		if (flags)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(ref_cast<ImageView>(m_RenderingInfo.pDepthAttachment->imageView)->m_DSVDescHandle, flags, m_RenderingInfo.pDepthAttachment->clearValue.depthStencil.depth, shrink_uint32_t_to_uint8_t(m_RenderingInfo.pStencilAttachment->clearValue.depthStencil.stencil), 0, nullptr);
	}
}

void CommandBuffer::EndRendering(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	for (auto& colourAttachment : m_RenderingInfo.colourAttachments)
	{
		if (colourAttachment.resolveImageView)
		{
			const Ref<crossplatform::Image>& colourImage = ref_cast<ImageView>(colourAttachment.imageView)->GetCreateInfo().pImage;
			const Ref<crossplatform::Image>& resolveImage = ref_cast<ImageView>(colourAttachment.resolveImageView)->GetCreateInfo().pImage;
			const crossplatform::Image::CreateInfo& colourImageCI = colourImage->GetCreateInfo();
			const crossplatform::Image::CreateInfo& resolveImageCI = resolveImage->GetCreateInfo();

			Image::Resolve resolveRegion;
			resolveRegion.srcSubresource = { crossplatform::Image::AspectBit::COLOUR_BIT, 0, 0, colourImageCI.arrayLayers };
			resolveRegion.srcOffset = { 0, 0, 0 };
			resolveRegion.dstSubresource = { crossplatform::Image::AspectBit::COLOUR_BIT, 0, 0, resolveImageCI.arrayLayers };
			resolveRegion.dstOffset = { 0, 0, 0 };
			resolveRegion.extent = { colourImageCI.width, colourImageCI.height, colourImageCI.depth };

			ResolveImage(index, colourImage, colourAttachment.imageLayout, resolveImage, colourAttachment.resolveImageLayout, { resolveRegion });
		}
	}
}

void CommandBuffer::BindPipeline(uint32_t index, const Ref<crossplatform::Pipeline>& pipeline) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::GRAPHICS)
	{
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetPipelineState(ref_cast<Pipeline>(pipeline)->m_Pipeline);
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetGraphicsRootSignature(ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootSignature);
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetViewports(static_cast<UINT>(ref_cast<Pipeline>(pipeline)->m_Viewports.size()), ref_cast<Pipeline>(pipeline)->m_Viewports.data());
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetScissorRects(static_cast<UINT>(ref_cast<Pipeline>(pipeline)->m_Scissors.size()), ref_cast<Pipeline>(pipeline)->m_Scissors.data());
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetPrimitiveTopology(Pipeline::ToD3D12_PRIMITIVE_TOPOLOGY(ref_cast<Pipeline>(pipeline)->GetCreateInfo().inputAssemblyState.topology));
		
		if (pipeline->GetCreateInfo().renderPass && !pipeline->GetCreateInfo().renderPass->GetCreateInfo().multiview.viewMasks.empty())
			reinterpret_cast<ID3D12GraphicsCommandList2*>(m_CmdBuffers[index])->SetViewInstanceMask(pipeline->GetCreateInfo().renderPass->GetCreateInfo().multiview.viewMasks[m_SubpassIndex]);
		else if (m_RenderingInfo.viewMask > 0)
			reinterpret_cast<ID3D12GraphicsCommandList2*>(m_CmdBuffers[index])->SetViewInstanceMask(m_RenderingInfo.viewMask);
	}
	else if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::COMPUTE)
	{
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetPipelineState(ref_cast<Pipeline>(pipeline)->m_Pipeline);
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootSignature(ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootSignature);
	}
	else if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::RAY_TRACING)
	{
		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->SetPipelineState1(ref_cast<Pipeline>(pipeline)->m_RayTracingPipeline);
		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->SetComputeRootSignature(ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootSignature);
	}
	else
	{
		MIRU_ASSERT(true, "ERROR: D3D12: Unknown PipelineType.")
	}

};

void CommandBuffer::BindVertexBuffers(uint32_t index, const std::vector<Ref<crossplatform::BufferView>>& vertexBufferViews) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<D3D12_VERTEX_BUFFER_VIEW>vbvs;
	vbvs.reserve(vertexBufferViews.size());
	for (auto& vbv : vertexBufferViews)
		vbvs.push_back(ref_cast<BufferView>(vbv)->m_VBVDesc);

	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetVertexBuffers(0, static_cast<UINT>(vbvs.size()), vbvs.data());

};
void CommandBuffer::BindIndexBuffer(uint32_t index, const Ref<crossplatform::BufferView>& indexBufferView) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetIndexBuffer(&ref_cast<BufferView>(indexBufferView)->m_IBVDesc);
};

void CommandBuffer::BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, uint32_t firstSet, const Ref<crossplatform::Pipeline>& pipeline)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	if (m_SetDescriptorHeaps_PerCmdBuffer[index])
	{
		ID3D12DescriptorHeap* heaps[2] = { m_CmdBuffer_CBV_SRV_UAV_DescriptorHeap,  m_CmdBuffer_Sampler_DescriptorHeap };
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetDescriptorHeaps(2, heaps);
		m_SetDescriptorHeaps_PerCmdBuffer[index] = false;
	}
	
	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT samplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	
	MIRU_ASSERT(!(m_CmdBuffer_CBV_SRV_UAV_DescriptorOffset < m_MaxDescriptorCount * cbv_srv_uav_DescriptorSize), "ERROR: D3D12: Exceeded maximum Descriptor count for type CBV_SRV_UAV.");
	MIRU_ASSERT(!(m_CmdBuffer_SamplerDescriptorOffset < m_MaxSamplerCount * samplerDescriptorSize), "ERROR: D3D12: Exceeded maximum Descriptor count for type SAMPLER.");

	UINT Current_CBV_SRV_UAV_DescriptorOffset = 0;
	UINT Current_Sampler_DescriptorOffset = 0;
	
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> CBV_SRV_UAV_GPUDescriptorHandles;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> Sampler_GPUDescriptorHandles;

	UINT totalDescriptorSets = 0;

	for(auto& descriptorSet : descriptorSets)
	{
		Ref<DescriptorSet> d3d12DescriptorSet = ref_cast<DescriptorSet>(descriptorSet);
		const auto& heap = d3d12DescriptorSet->m_DescriptorHeaps;
		const auto& heapDesc = d3d12DescriptorSet->m_DescriptorHeapDescs;
		totalDescriptorSets += static_cast<UINT>(descriptorSet->GetCreateInfo().pDescriptorSetLayouts.size());
		
		for (size_t i = 0; i < heap.size(); i++)
		{
			if (heapDesc[i][0].NumDescriptors)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE Current_CmdBuffer_CBV_SRV_UAV_CPUDescriptorHandle;
				Current_CmdBuffer_CBV_SRV_UAV_CPUDescriptorHandle.ptr = m_CmdBuffer_CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + Current_CBV_SRV_UAV_DescriptorOffset + m_CmdBuffer_CBV_SRV_UAV_DescriptorOffset;
				CBV_SRV_UAV_GPUDescriptorHandles.push_back({ m_CmdBuffer_CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + Current_CBV_SRV_UAV_DescriptorOffset + m_CmdBuffer_CBV_SRV_UAV_DescriptorOffset });
				m_Device->CopyDescriptorsSimple(heapDesc[i][0].NumDescriptors, Current_CmdBuffer_CBV_SRV_UAV_CPUDescriptorHandle, heap[i][0]->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				Current_CBV_SRV_UAV_DescriptorOffset += heapDesc[i][0].NumDescriptors * cbv_srv_uav_DescriptorSize;
			}

			if (heapDesc[i][1].NumDescriptors)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE Current_CmdBuffer_Sampler_CPUDescriptorHandle;
				Current_CmdBuffer_Sampler_CPUDescriptorHandle.ptr = m_CmdBuffer_Sampler_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + Current_Sampler_DescriptorOffset + m_CmdBuffer_SamplerDescriptorOffset;
				Sampler_GPUDescriptorHandles.push_back({ m_CmdBuffer_Sampler_DescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + Current_Sampler_DescriptorOffset + m_CmdBuffer_SamplerDescriptorOffset });
				m_Device->CopyDescriptorsSimple(heapDesc[i][1].NumDescriptors, Current_CmdBuffer_Sampler_CPUDescriptorHandle, heap[i][1]->GetCPUDescriptorHandleForHeapStart(),D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				Current_Sampler_DescriptorOffset += heapDesc[i][1].NumDescriptors * samplerDescriptorSize;
			}
		}
	}

	m_CmdBuffer_CBV_SRV_UAV_DescriptorOffset += Current_CBV_SRV_UAV_DescriptorOffset;
	m_CmdBuffer_SamplerDescriptorOffset += Current_Sampler_DescriptorOffset;
	
	size_t CBV_SRV_UAV_GPUDescriptorHandleIndex = 0;
	size_t Sampler_GPUDescriptorHandleIndex = 0;

	size_t rootParameterIndex = 0;
	for (const auto& rootParameter : ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootParameters)
	{
		const D3D12_ROOT_DESCRIPTOR_TABLE& descriptorTable = rootParameter.DescriptorTable;
		D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle;

		const UINT& descriptorRangeSet = descriptorTable.pDescriptorRanges[0].RegisterSpace;
		UINT lastSet = firstSet + totalDescriptorSets - 1;
		if (descriptorRangeSet < firstSet || descriptorRangeSet > lastSet)
		{
			rootParameterIndex++;
			continue;
		}
	
		if (descriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
		{
			MIRU_ASSERT(!(Sampler_GPUDescriptorHandleIndex < Sampler_GPUDescriptorHandles.size()), "ERROR: D3D12: No D3D12_GPU_DESCRIPTOR_HANDLE is available.");
			GPUDescriptorHandle = Sampler_GPUDescriptorHandles[Sampler_GPUDescriptorHandleIndex];
			Sampler_GPUDescriptorHandleIndex++;
		}
		else
		{
			MIRU_ASSERT(!(CBV_SRV_UAV_GPUDescriptorHandleIndex < CBV_SRV_UAV_GPUDescriptorHandles.size()), "ERROR: D3D12: No D3D12_GPU_DESCRIPTOR_HANDLE is available.");
			GPUDescriptorHandle = CBV_SRV_UAV_GPUDescriptorHandles[CBV_SRV_UAV_GPUDescriptorHandleIndex];
			CBV_SRV_UAV_GPUDescriptorHandleIndex++;
		}

		if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::GRAPHICS)
		{
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetGraphicsRootDescriptorTable(static_cast<UINT>(rootParameterIndex), GPUDescriptorHandle);
		}
		else if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::COMPUTE)
		{
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootDescriptorTable(static_cast<UINT>(rootParameterIndex), GPUDescriptorHandle);
		}
		else if (pipeline->GetCreateInfo().type == crossplatform::PipelineType::RAY_TRACING)
		{
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootDescriptorTable(static_cast<UINT>(rootParameterIndex), GPUDescriptorHandle);
		}
		else
		{
			MIRU_ASSERT(true, "ERROR: D3D12: Unknown PipelineType.")
		}

		rootParameterIndex++;
	}
};

void CommandBuffer::DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
};

void CommandBuffer::Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
};

void CommandBuffer::Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::BuildAccelerationStructure(uint32_t index, const std::vector<Ref<crossplatform::AccelerationStructureBuildInfo>>& buildGeometryInfos, const std::vector<std::vector<crossplatform::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	for (auto& buildGeometryInfo : buildGeometryInfos)
	{
		const AccelerationStructureBuildInfo::BuildGeometryInfo& bgi = buildGeometryInfo->GetBuildGeometryInfo();

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
		desc.DestAccelerationStructureData = bgi.dstAccelerationStructure ? static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(GetAccelerationStructureDeviceAddress(m_CI.pCommandPool->GetCreateInfo().pContext->GetDevice(), bgi.dstAccelerationStructure)) : D3D12_GPU_VIRTUAL_ADDRESS(0);
		desc.Inputs = ref_cast<AccelerationStructureBuildInfo>(buildGeometryInfo)->m_BRASI;
		desc.SourceAccelerationStructureData = bgi.srcAccelerationStructure ? static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(GetAccelerationStructureDeviceAddress(m_CI.pCommandPool->GetCreateInfo().pContext->GetDevice(), bgi.srcAccelerationStructure)) : D3D12_GPU_VIRTUAL_ADDRESS(0);
		desc.ScratchAccelerationStructureData = static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(bgi.scratchData.deviceAddress);
		
		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = ref_cast<Buffer>(bgi.dstAccelerationStructure->GetCreateInfo().buffer)->m_Buffer;
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ResourceBarrier(1, &barrier);
	}
}

void CommandBuffer::TraceRays(uint32_t index, const crossplatform::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pMissShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pHitShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	D3D12_DISPATCH_RAYS_DESC desc;

	if (pRaygenShaderBindingTable)
		desc.RayGenerationShaderRecord = { pRaygenShaderBindingTable->deviceAddress, pRaygenShaderBindingTable->size };
	else
		desc.RayGenerationShaderRecord = { 0, 0 };

	if (pMissShaderBindingTable)
		desc.MissShaderTable = { pMissShaderBindingTable->deviceAddress, pMissShaderBindingTable->size, pMissShaderBindingTable->stride };
	else
		desc.MissShaderTable = { 0, 0, 0 };

	if (pHitShaderBindingTable)
		desc.HitGroupTable = { pHitShaderBindingTable->deviceAddress, pHitShaderBindingTable->size, pHitShaderBindingTable->stride };
	else
		desc.HitGroupTable = { 0, 0, 0 };

	if (pCallableShaderBindingTable)
		desc.CallableShaderTable = { pCallableShaderBindingTable->deviceAddress, pCallableShaderBindingTable->size, pCallableShaderBindingTable->stride };
	else
		desc.CallableShaderTable = { 0, 0, 0 };

	desc.Width = static_cast<UINT>(width);
	desc.Height = static_cast<UINT>(height);
	desc.Depth = static_cast<UINT>(depth);
	reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->DispatchRays(&desc);
}

void CommandBuffer::CopyBuffer(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Buffer>& dstBuffer, const std::vector<crossplatform::Buffer::Copy>& copyRegions) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	for (auto& copyRegion : copyRegions)
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyBufferRegion(
			ref_cast<Buffer>(dstBuffer)->m_Buffer, static_cast<UINT>(copyRegion.dstOffset), 
			ref_cast<Buffer>(srcBuffer)->m_Buffer, static_cast<UINT>(copyRegion.srcOffset), static_cast<UINT>(copyRegion.size));
};

void CommandBuffer::CopyImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Copy>& copyRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

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

		for (uint32_t i = 0; i < copyRegion.dstSubresource.arrayLayerCount; i++)
		{
			const D3D12_RESOURCE_DESC& dstResDesc = dst.pResource->GetDesc();
			const D3D12_RESOURCE_DESC& srcResDesc = src.pResource->GetDesc();
			dst.SubresourceIndex = Image::D3D12CalculateSubresource(copyRegion.dstSubresource.mipLevel, i + copyRegion.dstSubresource.baseArrayLayer, 0, dstResDesc.MipLevels, dstResDesc.DepthOrArraySize);
			src.SubresourceIndex = Image::D3D12CalculateSubresource(copyRegion.srcSubresource.mipLevel, i + copyRegion.srcSubresource.baseArrayLayer, 0, srcResDesc.MipLevels, srcResDesc.DepthOrArraySize);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, copyRegion.dstOffset.x, copyRegion.dstOffset.y, copyRegion.dstOffset.z, &src, &srcbox);
		}
	}
};

void CommandBuffer::CopyBufferToImage(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions)
{
	MIRU_CPU_PROFILE_FUNCTION();

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
		const D3D12_RESOURCE_DESC& dstResDesc = dst.pResource->GetDesc();
		m_Device->GetCopyableFootprints(&dstResDesc, 0, 1, region.bufferOffset, &Layout, &NumRows, &RowSizesInBytes, &RequiredSize);
		src.PlacedFootprint = Layout;

		for (uint32_t i = 0; i < region.imageSubresource.arrayLayerCount; i++)
		{
			dst.SubresourceIndex = Image::D3D12CalculateSubresource(region.imageSubresource.mipLevel, i + region.imageSubresource.baseArrayLayer, 0, dstResDesc.MipLevels, dstResDesc.DepthOrArraySize);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z, &src, nullptr);
		}
	}	
}

void CommandBuffer::CopyImageToBuffer(uint32_t index, const Ref<crossplatform::Image>& srcImage, const Ref<crossplatform::Buffer>& dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions)
{
	MIRU_CPU_PROFILE_FUNCTION();

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
		const D3D12_RESOURCE_DESC& srcResDesc = src.pResource->GetDesc();
		m_Device->GetCopyableFootprints(&srcResDesc, 0, 1, region.bufferOffset, &Layout, &NumRows, &RowSizesInBytes, &RequiredSize);
		dst.PlacedFootprint = Layout;

		for (uint32_t i = 0; i < region.imageSubresource.arrayLayerCount; i++)
		{
			src.SubresourceIndex = Image::D3D12CalculateSubresource(region.imageSubresource.mipLevel, i + region.imageSubresource.baseArrayLayer, 0, srcResDesc.MipLevels, srcResDesc.DepthOrArraySize);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z, &src, nullptr);
		}
	}
}

void CommandBuffer::ResolveImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Resolve>& resolveRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	Barrier::CreateInfo bCI;
	for (auto& resolveRegion : resolveRegions)
	{
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
		bCI.dstAccess = Barrier::AccessBit::NONE_BIT;
		bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		bCI.pImage = srcImage;
		bCI.oldLayout = srcImageLayout;
		bCI.newLayout = Image::Layout::D3D12_RESOLVE_SOURCE;
		bCI.subresourceRange = { resolveRegion.srcSubresource.aspectMask, resolveRegion.srcSubresource.mipLevel, 1, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
		Ref<crossplatform::Barrier> preResolveBarrierSrc = Barrier::Create(&bCI);
		bCI.pImage = dstImage;
		bCI.oldLayout = dstImageLayout;
		bCI.newLayout = Image::Layout::D3D12_RESOLVE_DEST;
		bCI.subresourceRange = { resolveRegion.dstSubresource.aspectMask, resolveRegion.dstSubresource.mipLevel, 1, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
		Ref<crossplatform::Barrier> preResolveBarrierDst = Barrier::Create(&bCI);
		PipelineBarrier(index, crossplatform::PipelineStageBit::FRAGMENT_SHADER_BIT, crossplatform::PipelineStageBit::TRANSFER_BIT, crossplatform::DependencyBit::NONE_BIT, { preResolveBarrierSrc, preResolveBarrierDst });

		D3D12_RECT srcRect = {};
		srcRect.left = static_cast<UINT>(resolveRegion.srcOffset.x);
		srcRect.top = static_cast<UINT>(resolveRegion.srcOffset.y);
		srcRect.right = static_cast<UINT>(resolveRegion.extent.width);
		srcRect.bottom = static_cast<UINT>(resolveRegion.extent.height);

		bool formatCheck = srcImage->GetCreateInfo().format == dstImage->GetCreateInfo().format;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		if (formatCheck)
		{
			format = ref_cast<Image>(srcImage)->m_ResourceDesc.Format;
		}
		else
		{
			MIRU_ASSERT(true, "ERROR: D3D12: Source and Destination formats for resolve images must match.");
		}

		bool arrayCheck = resolveRegion.srcSubresource.arrayLayerCount == resolveRegion.dstSubresource.arrayLayerCount;
		if (arrayCheck)
		{
			for (uint32_t i = 0; i < resolveRegion.srcSubresource.arrayLayerCount; i++)
			{
				UINT dstSubresoucre = Image::D3D12CalculateSubresource(resolveRegion.dstSubresource.mipLevel, i + resolveRegion.srcSubresource.baseArrayLayer, 0, dstImage->GetCreateInfo().mipLevels, dstImage->GetCreateInfo().arrayLayers);
				UINT srcSubresoucre = Image::D3D12CalculateSubresource(resolveRegion.srcSubresource.mipLevel, i + resolveRegion.dstSubresource.baseArrayLayer, 0, srcImage->GetCreateInfo().mipLevels, srcImage->GetCreateInfo().arrayLayers);

				reinterpret_cast<ID3D12GraphicsCommandList1*>(m_CmdBuffers[index])->ResolveSubresourceRegion(
					ref_cast<Image>(dstImage)->m_Image, dstSubresoucre, static_cast<UINT>(resolveRegion.dstOffset.x), static_cast<UINT>(resolveRegion.dstOffset.y),
					ref_cast<Image>(srcImage)->m_Image, srcSubresoucre, &srcRect, format, D3D12_RESOLVE_MODE_AVERAGE);
			}
		}
		else
		{
			MIRU_ASSERT(true, "ERROR: D3D12: Source and Destination arrayLayerCount for resolve image subresources must match.");
		}

		bCI.pImage = srcImage;
		bCI.oldLayout = Image::Layout::D3D12_RESOLVE_SOURCE;
		bCI.newLayout = srcImageLayout;
		bCI.subresourceRange = { resolveRegion.srcSubresource.aspectMask, resolveRegion.srcSubresource.mipLevel, 1, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
		Ref<crossplatform::Barrier> postResolveBarrierSrc = Barrier::Create(&bCI);
		bCI.pImage = dstImage;
		bCI.oldLayout = Image::Layout::D3D12_RESOLVE_DEST;
		bCI.newLayout = dstImageLayout;
		bCI.subresourceRange = { resolveRegion.dstSubresource.aspectMask, resolveRegion.dstSubresource.mipLevel, 1, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
		Ref<crossplatform::Barrier> postResolveBarrierDst = Barrier::Create(&bCI);
		PipelineBarrier(index, crossplatform::PipelineStageBit::TRANSFER_BIT, crossplatform::PipelineStageBit::TRANSFER_BIT, crossplatform::DependencyBit::NONE_BIT, { postResolveBarrierSrc, postResolveBarrierDst });
	}
}

void CommandBuffer::ResolvePreviousSubpassAttachments(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	if (m_SubpassIndex == MIRU_SUBPASS_EXTERNAL)
		return;
	
	const Ref<crossplatform::RenderPass>& renderPass = m_RenderPassFramebuffer->GetCreateInfo().renderPass;
	const RenderPass::SubpassDescription& subpassDesc = renderPass->GetCreateInfo().subpassDescriptions[m_SubpassIndex];
	const std::vector<Ref<crossplatform::ImageView>>& framebufferAttachments = m_RenderPassFramebuffer->GetCreateInfo().attachments;
	const std::vector<crossplatform::RenderPass::AttachmentDescription>& renderpassAttachments = renderPass->GetCreateInfo().attachments;

	if (subpassDesc.resolveAttachments.empty())
		return;

	if (subpassDesc.colourAttachments.size() < subpassDesc.resolveAttachments.size())
	{
		MIRU_ASSERT(true, "ERROR: D3D12: More resolve attachment provide than colour attachements.");
	}

	for (size_t i = 0; i < subpassDesc.resolveAttachments.size(); i++)
	{
		const crossplatform::RenderPass::AttachmentReference& colour = subpassDesc.colourAttachments[i];
		const crossplatform::RenderPass::AttachmentReference& resolve = subpassDesc.resolveAttachments[i];

		const Ref<crossplatform::Image>& colourImage = framebufferAttachments[colour.attachmentIndex]->GetCreateInfo().pImage;
		const Ref<crossplatform::Image>& resolveImage = framebufferAttachments[resolve.attachmentIndex]->GetCreateInfo().pImage;
		const crossplatform::Image::CreateInfo& colourImageCI = colourImage->GetCreateInfo();
		const crossplatform::Image::CreateInfo& resolveImageCI = resolveImage->GetCreateInfo();

		Image::Resolve resolveRegion;
		resolveRegion.srcSubresource = {crossplatform::Image::AspectBit::COLOUR_BIT, 0, 0, colourImageCI.arrayLayers};
		resolveRegion.srcOffset = { 0, 0, 0 };
		resolveRegion.dstSubresource = { crossplatform::Image::AspectBit::COLOUR_BIT, 0, 0, resolveImageCI.arrayLayers };
		resolveRegion.dstOffset = { 0, 0, 0 };
		resolveRegion.extent = { colourImageCI.width, colourImageCI.height, colourImageCI.depth};

		ResolveImage(index, colourImage, m_RenderPassFramebufferAttachementLayouts[colour.attachmentIndex], resolveImage, m_RenderPassFramebufferAttachementLayouts[resolve.attachmentIndex], { resolveRegion });
	}
}

void CommandBuffer::BeginDebugLabel(uint32_t index, const std::string& label, std::array<float, 4> rgba)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	BYTE r = static_cast<BYTE>(std::clamp(static_cast<float>(0xFF) * rgba[0], 0.0f, 255.0f));
	BYTE g = static_cast<BYTE>(std::clamp(static_cast<float>(0xFF) * rgba[1], 0.0f, 255.0f));
	BYTE b = static_cast<BYTE>(std::clamp(static_cast<float>(0xFF) * rgba[2], 0.0f, 255.0f));
	if (PIXBeginEventOnCommandList)
		PIXBeginEventOnCommandList(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index]), PIX_COLOR(r, g, b), label.c_str());
}

void CommandBuffer::EndDebugLabel(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	if (PIXEndEventOnCommandList)
		PIXEndEventOnCommandList(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index]));
}
#endif