#include "D3D12CommandPoolBuffer.h"
#include "D3D12Device.h"
#include "D3D12Sync.h"
#include "D3D12Buffer.h"
#include "D3D12Image.h"
#include "D3D12Pipeline.h"
#include "D3D12DescriptorPoolSet.h"
#include "D3D12AccelerationStructure.h"
#include "D3D12Query.h"

#include "Include/WinPixEventRuntime/pix3.h"

using namespace miru;
using namespace d3d12;

//CmdPool
CommandPool::CommandPool(CommandPool::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Device>(pCreateInfo->device)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Queue = ref_cast<Device>(pCreateInfo->device)->m_Queues[GetCommandQueueIndex(pCreateInfo->queueType)];
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

	if (arc::BitwiseCheck(m_CI.flags, FlagBit::RESET_COMMAND_BUFFER_BIT))
	{
		for (auto& cmdPool : m_CmdPools)
		{
			MIRU_FATAL(cmdPool->Reset(), "ERROR: D3D12: Failed to reset CommandPool.");
		}
	}
}

uint32_t CommandPool::GetCommandQueueIndex(const CommandPool::QueueType& type)
{
	uint32_t index = 0;
	for (auto& queueDesc : ref_cast<Device>(m_CI.device)->m_QueueDescs)
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

	CommandPoolRef cmdPool = ref_cast<CommandPool>(m_CI.commandPool);
	m_Device = cmdPool->m_Device;
	D3D12_COMMAND_QUEUE_DESC queueDesc = cmdPool->m_Queue->GetDesc();
	std::vector<ID3D12CommandAllocator*>& d3d12CmdAllocators = cmdPool->m_CmdPools;

	d3d12CmdAllocators.resize(m_CI.commandBufferCount);
	m_CmdBuffers.resize(m_CI.commandBufferCount);
	m_RenderingResources.resize(m_CI.commandBufferCount);
	for (size_t i = 0; i < m_CmdBuffers.size(); i++)
	{
		MIRU_FATAL(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&d3d12CmdAllocators[i])), "ERROR: D3D12: Failed to create CommandPool.");
		D3D12SetName(d3d12CmdAllocators[i], cmdPool->GetCreateInfo().debugName + ": " + std::to_string(i));
	
		MIRU_FATAL(m_Device->CreateCommandList(0, m_CI.level == Level::SECONDARY ? D3D12_COMMAND_LIST_TYPE_BUNDLE : queueDesc.Type, d3d12CmdAllocators[i], nullptr, IID_PPV_ARGS(&m_CmdBuffers[i])), "ERROR: D3D12: Failed to create CommandBuffer.");
		D3D12SetName(m_CmdBuffers[i], m_CI.debugName + ": " + std::to_string(i));
		End(static_cast<uint32_t>(i));
	}

	switch (ref_cast<Device>(m_CI.commandPool->GetCreateInfo().device)->m_Features.d3d12Options.ResourceBindingTier)
	{
	case D3D12_RESOURCE_BINDING_TIER_3:
	{
		m_ResourceBindingCapabilities = { 1000000, 1000000, 1000000, 1000000, 1000000, 2048 };
		break;
	}
	case D3D12_RESOURCE_BINDING_TIER_2:
	{
		m_ResourceBindingCapabilities = { 1000000, 14, 1000000, 64, 1000000, 2048 };
		break;
	}
	case D3D12_RESOURCE_BINDING_TIER_1:
	default:
	{
		const auto& deviceRI = m_CI.commandPool->GetCreateInfo().device->GetResultInfo();
		uint32_t maxUAVsPerStage = (deviceRI.apiVersionMajor == 11 && deviceRI.apiVersionMinor == 0) ? 8 : 64;
		m_ResourceBindingCapabilities = { 1000000, 14, 128, maxUAVsPerStage, 16, 2048 };
		break;
	}
	}

	for (size_t i = 0; i < m_RenderingResources.size(); i++)
	{
		RenderingResource& renderingResource = m_RenderingResources[i];

		D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		DescriptorHeapDesc.NumDescriptors = m_ResourceBindingCapabilities.maxDescriptorCount;
		DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		DescriptorHeapDesc.NodeMask = 0;
		m_Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&renderingResource.CBV_SRV_UAV_DescriptorHeap));
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		DescriptorHeapDesc.NumDescriptors = m_ResourceBindingCapabilities.maxSamplerCount;
		m_Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&renderingResource.SAMPLER_DescriptorHeap));
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		DescriptorHeapDesc.NumDescriptors = m_ResourceBindingCapabilities.maxDescriptorCount * D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
		m_Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&renderingResource.RTV_DescriptorHeap));
		DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DescriptorHeapDesc.NumDescriptors = m_ResourceBindingCapabilities.maxDescriptorCount * 1;
		m_Device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&renderingResource.DSV_DescriptorHeap));

		renderingResource.CBV_SRV_UAV_DescriptorOffset = 0;
		renderingResource.SAMPLER_DescriptorOffset = 0;
		renderingResource.RTV_DescriptorOffset = 0;
		renderingResource.DSV_DescriptorOffset = 0;
	}
}

CommandBuffer::~CommandBuffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto& renderingResource : m_RenderingResources)
	{
		MIRU_D3D12_SAFE_RELEASE(renderingResource.CBV_SRV_UAV_DescriptorHeap);
		MIRU_D3D12_SAFE_RELEASE(renderingResource.SAMPLER_DescriptorHeap);
		MIRU_D3D12_SAFE_RELEASE(renderingResource.RTV_DescriptorHeap);
		MIRU_D3D12_SAFE_RELEASE(renderingResource.DSV_DescriptorHeap);
	}
	for (auto& cmdBuffer : m_CmdBuffers)
	{
		MIRU_D3D12_SAFE_RELEASE(cmdBuffer);
	}
}

void CommandBuffer::Begin(uint32_t index, UsageBit usage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	Reset(index, false);

	RenderingResource& renderingResource = m_RenderingResources[index];
	renderingResource.SetDescriptorHeap = true;
	renderingResource.CBV_SRV_UAV_DescriptorOffset = 0;
	renderingResource.SAMPLER_DescriptorOffset = 0;
	renderingResource.RTV_DescriptorOffset = 0;
	renderingResource.DSV_DescriptorOffset = 0;
}

void CommandBuffer::End(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_FATAL(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Close(), "ERROR: D3D12: Failed to end CommandBuffer.");
	m_RenderingResources[index].Resettable = true;
}

void CommandBuffer::Reset(uint32_t index, bool releaseResources)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	if (m_RenderingResources[index].Resettable)
	{
		std::vector<ID3D12CommandAllocator*>& d3d12CmdAllocators = ref_cast<CommandPool>(m_CI.commandPool)->m_CmdPools;

		MIRU_FATAL(d3d12CmdAllocators[index]->Reset(), "ERROR: D3D12: Failed to reset CommandPool.");
		MIRU_FATAL(reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Reset(d3d12CmdAllocators[index], nullptr), "ERROR: D3D12: Failed to reset CommandBuffer.");
		m_RenderingResources[index].Resettable = false;
	}
}

void CommandBuffer::ExecuteSecondaryCommandBuffers(uint32_t index, const base::CommandBufferRef& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices)
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

void CommandBuffer::Submit(const std::vector<base::CommandBuffer::SubmitInfo>& submitInfos, const base::FenceRef& fence)
{
	MIRU_CPU_PROFILE_FUNCTION();

	ID3D12CommandQueue* queue = ref_cast<CommandPool>(m_CI.commandPool)->m_Queue;
	std::vector<ID3D12CommandList*>submitCmdBuffers;

	for (const auto& submitInfo : submitInfos)
	{
		size_t waitIndex = 0;
		for (auto& wait : submitInfo.waits)
		{
			uint64_t value = 0;
			if (wait->GetCreateInfo().type == Semaphore::Type::TIMELINE)
			{
				value = submitInfo.waitValues[waitIndex];
			}
			else
			{
				value = ref_cast<Semaphore>(wait)->GetValue();
			}
			waitIndex++;

			if (value > 0)
			{
				MIRU_FATAL(queue->Wait(ref_cast<Semaphore>(wait)->m_Semaphore, value), "ERROR: D3D12: Failed to Wait on the wait Semaphore.");
			}
		}

		for (auto& index : submitInfo.indices)
		{
			if (index < m_CI.commandBufferCount)
				submitCmdBuffers.push_back(m_CmdBuffers[index]);
		}
		queue->ExecuteCommandLists(static_cast<uint32_t>(submitCmdBuffers.size()), submitCmdBuffers.data());

		size_t signalIndex = 0;
		for (auto& signal : submitInfo.signals)
		{
			uint64_t value = 0;
			if (signal->GetCreateInfo().type == Semaphore::Type::TIMELINE)
			{
				value = submitInfo.signalValues[waitIndex];
			}
			else
			{
				ref_cast<Semaphore>(signal)->GetValue()++;
				value = ref_cast<Semaphore>(signal)->GetValue();
			}
			MIRU_FATAL(queue->Signal(ref_cast<Semaphore>(signal)->m_Semaphore, value), "ERROR: D3D12: Failed to Signal the signal Semaphore.");
			signalIndex++;
		}
	}

	if (fence)
	{
		ref_cast<Fence>(fence)->GetValue()++;
		MIRU_FATAL(queue->Signal(ref_cast<Fence>(fence)->m_Fence, ref_cast<Fence>(fence)->GetValue()), "ERROR: D3D12: Failed to Signal the draw Fence.");
	}
}

void CommandBuffer::Submit2(const std::vector<base::CommandBuffer::SubmitInfo2>& submitInfo2s, const base::FenceRef& fence)
{
	MIRU_CPU_PROFILE_FUNCTION();

	ID3D12CommandQueue* queue = ref_cast<CommandPool>(m_CI.commandPool)->m_Queue;
	std::vector<ID3D12CommandList*>submitCmdBuffers;

	for (const auto& submitInfo : submitInfo2s)
	{
		for (auto& waitSemaphoreInfo : submitInfo.waitSemaphoreInfos)
		{
			const base::SemaphoreRef& wait = waitSemaphoreInfo.semaphore;
			uint64_t value = 0;
			if (wait->GetCreateInfo().type == Semaphore::Type::TIMELINE)
			{
				value = waitSemaphoreInfo.value;
			}
			else
			{
				value = ref_cast<Semaphore>(wait)->GetValue();
			}

			if (value > 0)
			{
				MIRU_FATAL(queue->Wait(ref_cast<Semaphore>(wait)->m_Semaphore, value), "ERROR: D3D12: Failed to Wait on the wait Semaphore.");
			}
		}

		for (auto& commandBufferInfo : submitInfo.commandBufferInfos)
		{
			if (commandBufferInfo.index < m_CI.commandBufferCount)
				submitCmdBuffers.push_back(m_CmdBuffers[commandBufferInfo.index]);
		}
		queue->ExecuteCommandLists(static_cast<uint32_t>(submitCmdBuffers.size()), submitCmdBuffers.data());

		for (auto& signalSemaphoreInfo : submitInfo.signalSemaphoreInfos)
		{
			const base::SemaphoreRef& signal = signalSemaphoreInfo.semaphore;
			uint64_t value = 0;
			if (signal->GetCreateInfo().type == Semaphore::Type::TIMELINE)
			{
				value = signalSemaphoreInfo.value;
			}
			else
			{
				ref_cast<Semaphore>(signal)->GetValue()++;
				value = ref_cast<Semaphore>(signal)->GetValue();
			}
			MIRU_FATAL(queue->Signal(ref_cast<Semaphore>(signal)->m_Semaphore, value), "ERROR: D3D12: Failed to Signal the signal Semaphore.");
		}
	}

	if (fence)
	{
		ref_cast<Fence>(fence)->GetValue()++;
		MIRU_FATAL(queue->Signal(ref_cast<Fence>(fence)->m_Fence, ref_cast<Fence>(fence)->GetValue()), "ERROR: D3D12: Failed to Signal the draw Fence.");
	}
}

void CommandBuffer::SetEvent(uint32_t index, const base::EventRef& event, base::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandBuffer::ResetEvent(uint32_t index, const base::EventRef& event, base::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandBuffer::WaitEvents(uint32_t index, const std::vector<base::EventRef>& events, base::PipelineStageBit srcStage, base::PipelineStageBit dstStage, const std::vector<base::BarrierRef>& barriers)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void CommandBuffer::PipelineBarrier(uint32_t index, base::PipelineStageBit srcStage, base::PipelineStageBit dstStage, base::DependencyBit dependencies, const std::vector<base::BarrierRef>& barriers)
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

void CommandBuffer::PipelineBarrier2(uint32_t index, const base::CommandBuffer::DependencyInfo& dependencyInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<D3D12_BARRIER_GROUP> barrierGroups;
	for (auto& barrier : dependencyInfo.barriers)
	{
		D3D12_BARRIER_GROUP barrierGroup;
		if (barrier->GetCreateInfo().type == Barrier::Type::MEMORY)
		{
			const std::vector<D3D12_GLOBAL_BARRIER>& globalBarrier = ref_cast<Barrier2>(barrier)->m_GlobalBarriers;
			barrierGroup.Type = D3D12_BARRIER_TYPE_GLOBAL;
			barrierGroup.NumBarriers = static_cast<UINT>(globalBarrier.size());
			barrierGroup.pGlobalBarriers = globalBarrier.data();
		}
		else if (barrier->GetCreateInfo().type == Barrier::Type::BUFFER)
		{
			const std::vector<D3D12_BUFFER_BARRIER>& bufferBarrier = ref_cast<Barrier2>(barrier)->m_BufferBarriers;
			barrierGroup.Type = D3D12_BARRIER_TYPE_BUFFER;
			barrierGroup.NumBarriers = static_cast<UINT>(bufferBarrier.size());
			barrierGroup.pBufferBarriers = bufferBarrier.data();
		}
		else if (barrier->GetCreateInfo().type == Barrier::Type::IMAGE)
		{
			const std::vector<D3D12_TEXTURE_BARRIER>& textureBarrier = ref_cast<Barrier2>(barrier)->m_TextureBarriers;
			barrierGroup.Type = D3D12_BARRIER_TYPE_TEXTURE;
			barrierGroup.NumBarriers = static_cast<UINT>(textureBarrier.size());
			barrierGroup.pTextureBarriers = textureBarrier.data();
		}
		barrierGroups.push_back(barrierGroup);
	}
	if (barrierGroups.empty())
		return;

	reinterpret_cast<ID3D12GraphicsCommandList7*>(m_CmdBuffers[index])->Barrier(static_cast<UINT>(barrierGroups.size()), barrierGroups.data());
}

void CommandBuffer::ClearColourImage(uint32_t index, const base::ImageRef& image, base::Image::Layout layout, const base::Image::ClearColourValue& clear, const std::vector<base::Image::SubresourceRange>& subresourceRanges)
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
	MIRU_FATAL(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)), "ERROR: D3D12: Failed to create temporary DescriptorHeap for RenderTargetViews.");
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();

	UINT RTV_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (size_t h = 0; h < subresourceRanges.size(); h++)
	{
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
		{
			ImageView::CreateInfo imageViewCI;
			imageViewCI.debugName = "CommandBuffer::ClearColourImage RTV: " + std::to_string(h) + " MIP: " + std::to_string(i);
			imageViewCI.device = m_CI.commandPool->GetCreateInfo().device;
			imageViewCI.image = image;
			imageViewCI.viewType = image->GetCreateInfo().type;
			imageViewCI.subresourceRange = subresourceRanges[h];
			imageViewCI.subresourceRange.baseMipLevel = i;
			imageViewCI.subresourceRange.mipLevelCount = 1;
			d3d12::ImageViewRef rtv = ref_cast<d3d12::ImageView>(ImageView::Create(&imageViewCI));

			m_Device->CreateRenderTargetView(ref_cast<Image>(image)->m_Image, &(rtv->m_RTVDesc), handle);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(handle, clear.float32, 0, nullptr);
			handle.ptr += RTV_DescriptorSize;
		}
	}

	MIRU_D3D12_SAFE_RELEASE(heap);
}

void CommandBuffer::ClearDepthStencilImage(uint32_t index, const base::ImageRef& image, base::Image::Layout layout, const base::Image::ClearDepthStencilValue& clear, const std::vector<base::Image::SubresourceRange>& subresourceRanges)
{
	MIRU_CPU_PROFILE_FUNCTION();

	UINT descriptorCount = 0;
	for (size_t h = 0; h < subresourceRanges.size(); h++)
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
			descriptorCount++;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = descriptorCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ID3D12DescriptorHeap* heap;
	MIRU_FATAL(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)), "ERROR: D3D12: Failed to create temporary DescriptorHeap for DepthStencilViews.");
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();

	UINT DSV_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	for (size_t h = 0; h < subresourceRanges.size(); h++)
	{
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].baseMipLevel + subresourceRanges[h].mipLevelCount; i++)
		{
			ImageView::CreateInfo imageViewCI;
			imageViewCI.debugName = "CommandBuffer::ClearColourImage DSV: " + std::to_string(h) + " MIP: " + std::to_string(i);
			imageViewCI.device = m_CI.commandPool->GetCreateInfo().device;;
			imageViewCI.image = image;
			imageViewCI.viewType = image->GetCreateInfo().type;
			imageViewCI.subresourceRange = subresourceRanges[h];
			imageViewCI.subresourceRange.baseMipLevel = i;
			imageViewCI.subresourceRange.mipLevelCount = 1;
			d3d12::ImageViewRef dsv = ref_cast<d3d12::ImageView>(ImageView::Create(&imageViewCI));

			m_Device->CreateDepthStencilView(ref_cast<Image>(image)->m_Image, &(dsv->m_DSVDesc), handle);
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear.depth, static_cast<UINT8>(clear.stencil), 0, nullptr);
			handle.ptr += DSV_DescriptorSize;
		}
	}

	MIRU_D3D12_SAFE_RELEASE(heap);
}

void CommandBuffer::BeginRendering(uint32_t index, const base::RenderingInfo& renderingInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	RenderingResource& renderingResource = m_RenderingResources[index];
	renderingResource.RenderingInfo = renderingInfo;

	//Set RenderTargets
	UINT RTV_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT DSV_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
	for (auto& attachment : renderingResource.RenderingInfo.colourAttachments)
	{
		ImageViewRef imageView = ref_cast<ImageView>(attachment.imageView);
		ImageRef image = ref_cast<Image>(imageView->GetCreateInfo().image);
		D3D12_CPU_DESCRIPTOR_HANDLE& RTVDescHandle = imageView->m_RTVDescHandle;
		if (!RTVDescHandle.ptr)
		{
			RTVDescHandle.ptr = renderingResource.RTV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + renderingResource.RTV_DescriptorOffset;
			m_Device->CreateRenderTargetView(image->m_Image, &(imageView->m_RTVDesc), RTVDescHandle);
			renderingResource.RTV_DescriptorOffset += RTV_DescriptorSize;
		}
		rtvs.push_back(RTVDescHandle);

		if (attachment.resolveImageView)
		{
			ImageViewRef imageView = ref_cast<ImageView>(attachment.resolveImageView);
			ImageRef image = ref_cast<Image>(imageView->GetCreateInfo().image);
			D3D12_CPU_DESCRIPTOR_HANDLE& RTVDescHandle = imageView->m_RTVDescHandle;
			if (!RTVDescHandle.ptr)
			{
				RTVDescHandle.ptr = renderingResource.RTV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + renderingResource.RTV_DescriptorOffset;
				m_Device->CreateRenderTargetView(image->m_Image, &(imageView->m_RTVDesc), RTVDescHandle);
				renderingResource.RTV_DescriptorOffset += RTV_DescriptorSize;
			}
			// Resolve targets are not pushed back to rtvs for OMSetRenderTargets(), but RTVDescHandle is need for clearing the image.
		}
	}
	if (renderingResource.RenderingInfo.pDepthAttachment)
	{
		ImageViewRef imageView = ref_cast<ImageView>(renderingResource.RenderingInfo.pDepthAttachment->imageView);
		ImageRef image = ref_cast<Image>(imageView->GetCreateInfo().image);
		D3D12_CPU_DESCRIPTOR_HANDLE& DSVDescHandle = imageView->m_DSVDescHandle;
		if (!DSVDescHandle.ptr)
		{
			DSVDescHandle.ptr = renderingResource.DSV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + renderingResource.DSV_DescriptorOffset;
			m_Device->CreateDepthStencilView(image->m_Image, &(imageView->m_DSVDesc), DSVDescHandle);
			renderingResource.DSV_DescriptorOffset += DSV_DescriptorSize;
		}
		dsv = DSVDescHandle;
	}
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), false, (dsv.ptr ? &dsv : nullptr));

	//Clear image views
	uint32_t attachId = 0;
	for (auto& attachment : renderingResource.RenderingInfo.colourAttachments)
	{
		if (attachment.loadOp == base::AttachmentLoadOp::CLEAR)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(ref_cast<ImageView>(attachment.imageView)->m_RTVDescHandle, attachment.clearValue.colour.float32, 0, nullptr);

		// Always clear to initialise the image.
		if (attachment.resolveImageView)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(ref_cast<ImageView>(attachment.resolveImageView)->m_RTVDescHandle, attachment.clearValue.colour.float32, 0, nullptr);
	}
	if (renderingResource.RenderingInfo.pDepthAttachment || renderingResource.RenderingInfo.pStencilAttachment)
	{
		D3D12_CLEAR_FLAGS flags = (D3D12_CLEAR_FLAGS)0;
		FLOAT depthClearValue = 0.0f;
		UINT8 stencilClearValue = 0;

		if (renderingResource.RenderingInfo.pDepthAttachment && renderingResource.RenderingInfo.pDepthAttachment->loadOp == base::AttachmentLoadOp::CLEAR)
		{
			flags |= D3D12_CLEAR_FLAG_DEPTH;
			depthClearValue = renderingResource.RenderingInfo.pDepthAttachment->clearValue.depthStencil.depth;
		}
		if (renderingResource.RenderingInfo.pStencilAttachment && renderingResource.RenderingInfo.pStencilAttachment->loadOp == base::AttachmentLoadOp::CLEAR)
		{	
			flags |= D3D12_CLEAR_FLAG_STENCIL;
			stencilClearValue = static_cast<UINT8>(renderingResource.RenderingInfo.pStencilAttachment->clearValue.depthStencil.stencil);
		}

		if (flags)
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(ref_cast<ImageView>(renderingResource.RenderingInfo.pDepthAttachment->imageView)->m_DSVDescHandle, flags, depthClearValue, stencilClearValue, 0, nullptr);
	}
}

void CommandBuffer::EndRendering(uint32_t index)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	RenderingResource& renderingResource = m_RenderingResources[index];

	for (auto& colourAttachment : renderingResource.RenderingInfo.colourAttachments)
	{
		if (colourAttachment.resolveImageView)
		{
			const base::ImageRef& colourImage = ref_cast<ImageView>(colourAttachment.imageView)->GetCreateInfo().image;
			const base::ImageRef& resolveImage = ref_cast<ImageView>(colourAttachment.resolveImageView)->GetCreateInfo().image;
			const base::Image::CreateInfo& colourImageCI = colourImage->GetCreateInfo();
			const base::Image::CreateInfo& resolveImageCI = resolveImage->GetCreateInfo();

			Image::Resolve resolveRegion;
			resolveRegion.srcSubresource = { base::Image::AspectBit::COLOUR_BIT, 0, 0, colourImageCI.arrayLayers };
			resolveRegion.srcOffset = { 0, 0, 0 };
			resolveRegion.dstSubresource = { base::Image::AspectBit::COLOUR_BIT, 0, 0, resolveImageCI.arrayLayers };
			resolveRegion.dstOffset = { 0, 0, 0 };
			resolveRegion.extent = { colourImageCI.width, colourImageCI.height, colourImageCI.depth };

			ResolveImage(index, colourImage, colourAttachment.imageLayout, resolveImage, colourAttachment.resolveImageLayout, { resolveRegion });
		}

		ImageViewRef imageView = ref_cast<ImageView>(colourAttachment.imageView);
		if (!imageView->IsSwapchainImageView())
		{
			ImageRef image = ref_cast<Image>(imageView->GetCreateInfo().image);
			D3D12_CPU_DESCRIPTOR_HANDLE& RTVDescHandle = imageView->m_RTVDescHandle;
			RTVDescHandle.ptr = 0;
		}
	}
	if (renderingResource.RenderingInfo.pDepthAttachment)
	{
		ImageViewRef imageView = ref_cast<ImageView>(renderingResource.RenderingInfo.pDepthAttachment->imageView);
		if (!imageView->IsSwapchainImageView())
		{
			ImageRef image = ref_cast<Image>(imageView->GetCreateInfo().image);
			D3D12_CPU_DESCRIPTOR_HANDLE& DSVDescHandle = imageView->m_DSVDescHandle;
			DSVDescHandle.ptr = 0;
		}
	}
}

void CommandBuffer::BindPipeline(uint32_t index, const base::PipelineRef& pipeline) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	if (pipeline->GetCreateInfo().type == base::PipelineType::GRAPHICS)
	{
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetPipelineState(ref_cast<Pipeline>(pipeline)->m_Pipeline);
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetGraphicsRootSignature(ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootSignature);
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetPrimitiveTopology(Pipeline::ToD3D12_PRIMITIVE_TOPOLOGY(ref_cast<Pipeline>(pipeline)->GetCreateInfo().inputAssemblyState.topology));

		if (!arc::FindInVector(pipeline->GetCreateInfo().dynamicStates.dynamicStates, base::DynamicState::VIEWPORT))
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetViewports(static_cast<UINT>(ref_cast<Pipeline>(pipeline)->m_Viewports.size()), ref_cast<Pipeline>(pipeline)->m_Viewports.data());
		if (!arc::FindInVector(pipeline->GetCreateInfo().dynamicStates.dynamicStates, base::DynamicState::SCISSOR))
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetScissorRects(static_cast<UINT>(ref_cast<Pipeline>(pipeline)->m_Scissors.size()), ref_cast<Pipeline>(pipeline)->m_Scissors.data());
		
		/*if (pipeline->GetCreateInfo().renderPass && !pipeline->GetCreateInfo().renderPass->GetCreateInfo().multiview.viewMasks.empty())
			reinterpret_cast<ID3D12GraphicsCommandList1*>(m_CmdBuffers[index])->SetViewInstanceMask(pipeline->GetCreateInfo().renderPass->GetCreateInfo().multiview.viewMasks[m_RenderingResources[index].SubpassIndex]);
		else if (m_RenderingResources[index].RenderingInfo.viewMask > 0)
			reinterpret_cast<ID3D12GraphicsCommandList1*>(m_CmdBuffers[index])->SetViewInstanceMask(m_RenderingResources[index].RenderingInfo.viewMask);
		else
			reinterpret_cast<ID3D12GraphicsCommandList1*>(m_CmdBuffers[index])->SetViewInstanceMask(0);*/
	}
	else if (pipeline->GetCreateInfo().type == base::PipelineType::COMPUTE)
	{
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetPipelineState(ref_cast<Pipeline>(pipeline)->m_Pipeline);
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootSignature(ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootSignature);
	}
	else if (pipeline->GetCreateInfo().type == base::PipelineType::RAY_TRACING)
	{
		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->SetPipelineState1(ref_cast<Pipeline>(pipeline)->m_RayTracingPipeline);
		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->SetComputeRootSignature(ref_cast<Pipeline>(pipeline)->m_GlobalRootSignature.rootSignature);
	}
	else
	{
		MIRU_FATAL(true, "ERROR: D3D12: Unknown PipelineType.")
	}

};

void CommandBuffer::BindVertexBuffers(uint32_t index, const std::vector<base::BufferViewRef>& vertexBufferViews) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	std::vector<D3D12_VERTEX_BUFFER_VIEW>vbvs;
	vbvs.reserve(vertexBufferViews.size());
	for (auto& vbv : vertexBufferViews)
		vbvs.push_back(ref_cast<BufferView>(vbv)->m_VBVDesc);

	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetVertexBuffers(0, static_cast<UINT>(vbvs.size()), vbvs.data());

};
void CommandBuffer::BindIndexBuffer(uint32_t index, const base::BufferViewRef& indexBufferView) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->IASetIndexBuffer(&ref_cast<BufferView>(indexBufferView)->m_IBVDesc);
};

void CommandBuffer::BindDescriptorSets(uint32_t index, const std::vector<base::DescriptorSetRef>& descriptorSets, uint32_t firstSet, const base::PipelineRef& pipeline)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	RenderingResource& renderingResource = m_RenderingResources[index];

	if (renderingResource.SetDescriptorHeap)
	{
		ID3D12DescriptorHeap* heaps[2] = { renderingResource.CBV_SRV_UAV_DescriptorHeap,  renderingResource.SAMPLER_DescriptorHeap };
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetDescriptorHeaps(2, heaps);
		renderingResource.SetDescriptorHeap = false;
	}
	
	UINT CBV_SRV_UAV_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT SAMPLER_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	
	MIRU_FATAL(!(renderingResource.CBV_SRV_UAV_DescriptorOffset < m_ResourceBindingCapabilities.maxDescriptorCount * CBV_SRV_UAV_DescriptorSize), "ERROR: D3D12: Exceeded maximum Descriptor count for type CBV_SRV_UAV.");
	MIRU_FATAL(!(renderingResource.SAMPLER_DescriptorOffset < m_ResourceBindingCapabilities.maxSamplerCount * SAMPLER_DescriptorSize), "ERROR: D3D12: Exceeded maximum Descriptor count for type SAMPLER.");

	UINT Current_CBV_SRV_UAV_DescriptorOffset = 0;
	UINT Current_SAMPLER_DescriptorOffset = 0;
	
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> CBV_SRV_UAV_GPUDescriptorHandles;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> SAMPLER_GPUDescriptorHandles;

	UINT totalDescriptorSets = 0;

	for(auto& descriptorSet : descriptorSets)
	{
		DescriptorSetRef d3d12DescriptorSet = ref_cast<DescriptorSet>(descriptorSet);
		const auto& heap = d3d12DescriptorSet->m_DescriptorHeaps;
		const auto& heapDesc = d3d12DescriptorSet->m_DescriptorHeapDescs;
		totalDescriptorSets += static_cast<UINT>(descriptorSet->GetCreateInfo().descriptorSetLayouts.size());
		
		for (size_t i = 0; i < heap.size(); i++)
		{
			if (heapDesc[i][0].NumDescriptors)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE Current_CmdBuffer_CBV_SRV_UAV_CPUDescriptorHandle;
				Current_CmdBuffer_CBV_SRV_UAV_CPUDescriptorHandle.ptr = renderingResource.CBV_SRV_UAV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + Current_CBV_SRV_UAV_DescriptorOffset + renderingResource.CBV_SRV_UAV_DescriptorOffset;
				CBV_SRV_UAV_GPUDescriptorHandles.push_back({ renderingResource.CBV_SRV_UAV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + Current_CBV_SRV_UAV_DescriptorOffset + renderingResource.CBV_SRV_UAV_DescriptorOffset });
				m_Device->CopyDescriptorsSimple(heapDesc[i][0].NumDescriptors, Current_CmdBuffer_CBV_SRV_UAV_CPUDescriptorHandle, heap[i][0]->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				Current_CBV_SRV_UAV_DescriptorOffset += heapDesc[i][0].NumDescriptors * CBV_SRV_UAV_DescriptorSize;
			}

			if (heapDesc[i][1].NumDescriptors)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE Current_CmdBuffer_Sampler_CPUDescriptorHandle;
				Current_CmdBuffer_Sampler_CPUDescriptorHandle.ptr = renderingResource.SAMPLER_DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + Current_SAMPLER_DescriptorOffset + renderingResource.SAMPLER_DescriptorOffset;
				SAMPLER_GPUDescriptorHandles.push_back({ renderingResource.SAMPLER_DescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + Current_SAMPLER_DescriptorOffset + renderingResource.SAMPLER_DescriptorOffset });
				m_Device->CopyDescriptorsSimple(heapDesc[i][1].NumDescriptors, Current_CmdBuffer_Sampler_CPUDescriptorHandle, heap[i][1]->GetCPUDescriptorHandleForHeapStart(),D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				Current_SAMPLER_DescriptorOffset += heapDesc[i][1].NumDescriptors * SAMPLER_DescriptorSize;
			}
		}
	}

	renderingResource.CBV_SRV_UAV_DescriptorOffset += Current_CBV_SRV_UAV_DescriptorOffset;
	renderingResource.SAMPLER_DescriptorOffset += Current_SAMPLER_DescriptorOffset;
	
	size_t CBV_SRV_UAV_GPUDescriptorHandleIndex = 0;
	size_t SAMPLER_GPUDescriptorHandleIndex = 0;

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
			MIRU_FATAL(!(SAMPLER_GPUDescriptorHandleIndex < SAMPLER_GPUDescriptorHandles.size()), "ERROR: D3D12: No D3D12_GPU_DESCRIPTOR_HANDLE is available.");
			GPUDescriptorHandle = SAMPLER_GPUDescriptorHandles[SAMPLER_GPUDescriptorHandleIndex];
			SAMPLER_GPUDescriptorHandleIndex++;
		}
		else
		{
			MIRU_FATAL(!(CBV_SRV_UAV_GPUDescriptorHandleIndex < CBV_SRV_UAV_GPUDescriptorHandles.size()), "ERROR: D3D12: No D3D12_GPU_DESCRIPTOR_HANDLE is available.");
			GPUDescriptorHandle = CBV_SRV_UAV_GPUDescriptorHandles[CBV_SRV_UAV_GPUDescriptorHandleIndex];
			CBV_SRV_UAV_GPUDescriptorHandleIndex++;
		}

		if (pipeline->GetCreateInfo().type == base::PipelineType::GRAPHICS)
		{
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetGraphicsRootDescriptorTable(static_cast<UINT>(rootParameterIndex), GPUDescriptorHandle);
		}
		else if (pipeline->GetCreateInfo().type == base::PipelineType::COMPUTE)
		{
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootDescriptorTable(static_cast<UINT>(rootParameterIndex), GPUDescriptorHandle);
		}
		else if (pipeline->GetCreateInfo().type == base::PipelineType::RAY_TRACING)
		{
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->SetComputeRootDescriptorTable(static_cast<UINT>(rootParameterIndex), GPUDescriptorHandle);
		}
		else
		{
			MIRU_FATAL(true, "ERROR: D3D12: Unknown PipelineType.")
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

void CommandBuffer::DrawMeshTasks(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList6*>(m_CmdBuffers[index])->DispatchMesh(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::BuildAccelerationStructures(uint32_t index, const std::vector<base::AccelerationStructureBuildInfoRef>& buildGeometryInfos, const std::vector<std::vector<base::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	for (auto& buildGeometryInfo : buildGeometryInfos)
	{
		const AccelerationStructureBuildInfo::BuildGeometryInfo& bgi = buildGeometryInfo->GetBuildGeometryInfo();

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
		desc.DestAccelerationStructureData = bgi.dstAccelerationStructure ? static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(GetAccelerationStructureDeviceAddress(m_CI.commandPool->GetCreateInfo().device, bgi.dstAccelerationStructure)) : D3D12_GPU_VIRTUAL_ADDRESS(0);
		desc.Inputs = ref_cast<AccelerationStructureBuildInfo>(buildGeometryInfo)->m_BRASI;
		desc.SourceAccelerationStructureData = bgi.srcAccelerationStructure ? static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(GetAccelerationStructureDeviceAddress(m_CI.commandPool->GetCreateInfo().device, bgi.srcAccelerationStructure)) : D3D12_GPU_VIRTUAL_ADDRESS(0);
		desc.ScratchAccelerationStructureData = static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(bgi.scratchData.deviceAddress);
		
		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CmdBuffers[index])->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = ref_cast<Buffer>(bgi.dstAccelerationStructure->GetCreateInfo().buffer)->m_Buffer;
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ResourceBarrier(1, &barrier);
	}
}

void CommandBuffer::TraceRays(uint32_t index, const base::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const base::StridedDeviceAddressRegion* pMissShaderBindingTable, const base::StridedDeviceAddressRegion* pHitShaderBindingTable, const base::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
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

void CommandBuffer::CopyBuffer(uint32_t index, const base::BufferRef& srcBuffer, const base::BufferRef& dstBuffer, const std::vector<base::Buffer::Copy>& copyRegions) 
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	for (auto& copyRegion : copyRegions)
		reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyBufferRegion(
			ref_cast<Buffer>(dstBuffer)->m_Buffer, static_cast<UINT>(copyRegion.dstOffset), 
			ref_cast<Buffer>(srcBuffer)->m_Buffer, static_cast<UINT>(copyRegion.srcOffset), static_cast<UINT>(copyRegion.size));
};

void CommandBuffer::CopyImage(uint32_t index, const base::ImageRef& srcImage, base::Image::Layout srcImageLayout, const base::ImageRef& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::Copy>& copyRegions)
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

void CommandBuffer::CopyBufferToImage(uint32_t index, const base::BufferRef& srcBuffer, const base::ImageRef& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::BufferImageCopy>& regions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	for (auto& region : regions)
	{
		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = ref_cast<Buffer>(srcBuffer)->m_Buffer;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = ref_cast<Image>(dstImage)->m_Image;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layout;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		UINT64 RequiredSize;
		
		const D3D12_RESOURCE_DESC& dstResDesc = dst.pResource->GetDesc();

		for (uint32_t i = 0; i < region.imageSubresource.arrayLayerCount; i++)
		{
			UINT SubresourceIndex = Image::D3D12CalculateSubresource(region.imageSubresource.mipLevel, i + region.imageSubresource.baseArrayLayer, 0, dstResDesc.MipLevels, dstResDesc.DepthOrArraySize);
			m_Device->GetCopyableFootprints(&dstResDesc, SubresourceIndex, 1, region.bufferOffset, &Layout, &NumRows, &RowSizesInBytes, &RequiredSize);
			src.PlacedFootprint = Layout;
			dst.SubresourceIndex = SubresourceIndex;

			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z, &src, nullptr);
		}
	}	
}

void CommandBuffer::CopyImageToBuffer(uint32_t index, const base::ImageRef& srcImage, const base::BufferRef& dstBuffer, base::Image::Layout srcImageLayout, const std::vector<base::Image::BufferImageCopy>& regions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	for (auto& region : regions)
	{
		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = ref_cast<Image>(srcImage)->m_Image;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = ref_cast<Buffer>(dstBuffer)->m_Buffer;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		const D3D12_RESOURCE_DESC& srcResDesc = src.pResource->GetDesc();

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layout;
		UINT NumRows;
		UINT64 RowSizesInBytes;
		UINT64 RequiredSize;

		for (uint32_t i = 0; i < region.imageSubresource.arrayLayerCount; i++)
		{
			UINT SubresourceIndex = Image::D3D12CalculateSubresource(region.imageSubresource.mipLevel, i + region.imageSubresource.baseArrayLayer, 0, srcResDesc.MipLevels, srcResDesc.DepthOrArraySize);
			m_Device->GetCopyableFootprints(&srcResDesc, SubresourceIndex, 1, region.bufferOffset, &Layout, &NumRows, &RowSizesInBytes, &RequiredSize);
			src.SubresourceIndex = SubresourceIndex;
			dst.PlacedFootprint = Layout;

			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->CopyTextureRegion(&dst, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z, &src, nullptr);
		}
	}
}

void CommandBuffer::ResolveImage(uint32_t index, const base::ImageRef& srcImage, base::Image::Layout srcImageLayout, const base::ImageRef& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::Resolve>& resolveRegions)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	const RenderingResource& renderingResource = m_RenderingResources[index];

	base::ResolveModeBits resolveMode = base::ResolveModeBits::NONE_BIT;
	for (const auto& colourAttachment : renderingResource.RenderingInfo.colourAttachments)
	{
		bool srcSame = ref_cast<Image>(colourAttachment.imageView->GetCreateInfo().image)->m_Image == ref_cast<Image>(srcImage)->m_Image;
		bool dstSame = ref_cast<Image>(colourAttachment.resolveImageView->GetCreateInfo().image)->m_Image == ref_cast<Image>(dstImage)->m_Image;
		if (srcSame && dstSame)
			resolveMode = colourAttachment.resolveMode;
	}

	D3D12_RESOLVE_MODE d3d12ResolveMode = D3D12_RESOLVE_MODE(0);
	switch (resolveMode)
	{
	default:
	case base::ResolveModeBits::NONE_BIT:
		d3d12ResolveMode = D3D12_RESOLVE_MODE_DECOMPRESS;
		break;
	case base::ResolveModeBits::SAMPLE_ZERO_BIT:
		d3d12ResolveMode = D3D12_RESOLVE_MODE_MIN;
		break;
	case base::ResolveModeBits::AVERAGE_BIT:
		d3d12ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
		break;
	case base::ResolveModeBits::MIN_BIT:
		d3d12ResolveMode = D3D12_RESOLVE_MODE_MIN;
		break;
	case base::ResolveModeBits::MAX_BIT:
		d3d12ResolveMode = D3D12_RESOLVE_MODE_MAX;
		break;
	};

	auto D3D12CalculateSubresource = [](const base::Image::CreateInfo& createInfo, const base::Image::SubresourceLayers& subresourceLayer, UINT arrayLayerOffset) -> UINT
		{
			return Image::D3D12CalculateSubresource(subresourceLayer.mipLevel, subresourceLayer.baseArrayLayer + arrayLayerOffset, 0, createInfo.mipLevels, createInfo.arrayLayers);
		};

	const bool& useBarrier2 = arc::BitwiseCheck(m_CI.commandPool->GetCreateInfo().device->GetResultInfo().activeExtensions, base::Device::ExtensionsBit::SYNCHRONISATION_2);

	Barrier::CreateInfo bCI;
	Barrier2::CreateInfo b2CI;
	for (auto& resolveRegion : resolveRegions)
	{
		D3D12_RECT srcRect = {};
		srcRect.left = static_cast<UINT>(resolveRegion.srcOffset.x);
		srcRect.top = static_cast<UINT>(resolveRegion.srcOffset.y);
		srcRect.right = static_cast<UINT>(resolveRegion.extent.width);
		srcRect.bottom = static_cast<UINT>(resolveRegion.extent.height);

		if (useBarrier2)
		{
			b2CI.type = Barrier::Type::IMAGE;
			b2CI.srcStageMask = base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
			b2CI.srcAccess = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			b2CI.dstStageMask = base::PipelineStageBit::RESOLVE_BIT;
			b2CI.dstAccess = Barrier::AccessBit::D3D12_RESOLVE_SOURCE;
			b2CI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			b2CI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			b2CI.image = srcImage;
			b2CI.oldLayout = srcImageLayout;
			b2CI.newLayout = Image::Layout::D3D12_RESOLVE_SOURCE;
			b2CI.subresourceRange = { resolveRegion.srcSubresource.aspectMask, resolveRegion.srcSubresource.mipLevel, 1, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
			base::Barrier2Ref preResolveBarrierSrc = Barrier2::Create(&b2CI);

			b2CI.srcStageMask = base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
			b2CI.srcAccess = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			b2CI.dstStageMask = base::PipelineStageBit::RESOLVE_BIT;
			b2CI.dstAccess = Barrier::AccessBit::D3D12_RESOLVE_DEST;
			b2CI.image = dstImage;
			b2CI.oldLayout = dstImageLayout;
			b2CI.newLayout = Image::Layout::D3D12_RESOLVE_DEST;
			b2CI.subresourceRange = { resolveRegion.dstSubresource.aspectMask, resolveRegion.dstSubresource.mipLevel, 1, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
			base::Barrier2Ref preResolveBarrierDst = Barrier2::Create(&b2CI);

			PipelineBarrier2(index, { base::DependencyBit::NONE_BIT, { preResolveBarrierSrc, preResolveBarrierDst } });
		}
		else
		{
			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
			bCI.dstAccess = Barrier::AccessBit::NONE_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.image = srcImage;
			bCI.oldLayout = srcImageLayout;
			bCI.newLayout = Image::Layout::D3D12_RESOLVE_SOURCE;
			bCI.subresourceRange = { resolveRegion.srcSubresource.aspectMask, resolveRegion.srcSubresource.mipLevel, 1, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
			base::BarrierRef preResolveBarrierSrc = Barrier::Create(&bCI);

			bCI.image = dstImage;
			bCI.oldLayout = dstImageLayout;
			bCI.newLayout = Image::Layout::D3D12_RESOLVE_DEST;
			bCI.subresourceRange = { resolveRegion.dstSubresource.aspectMask, resolveRegion.dstSubresource.mipLevel, 1, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
			base::BarrierRef preResolveBarrierDst = Barrier::Create(&bCI);

			PipelineBarrier(index, base::PipelineStageBit::FRAGMENT_SHADER_BIT, base::PipelineStageBit::TRANSFER_BIT, base::DependencyBit::NONE_BIT, { preResolveBarrierSrc, preResolveBarrierDst });
		}

		bool formatCheck = srcImage->GetCreateInfo().format == dstImage->GetCreateInfo().format;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		if (formatCheck)
		{
			format = ref_cast<Image>(srcImage)->m_ResourceDesc.Format;
		}
		else
		{
			MIRU_FATAL(true, "ERROR: D3D12: Source and Destination formats for resolve images must match.");
		}

		bool arrayCheck = resolveRegion.srcSubresource.arrayLayerCount == resolveRegion.dstSubresource.arrayLayerCount;
		if (arrayCheck)
		{
			for (uint32_t i = 0; i < resolveRegion.srcSubresource.arrayLayerCount; i++)
			{
				UINT dstSubresoucre = D3D12CalculateSubresource(dstImage->GetCreateInfo(), resolveRegion.dstSubresource, i);
				UINT srcSubresoucre = D3D12CalculateSubresource(srcImage->GetCreateInfo(), resolveRegion.srcSubresource, i);

				reinterpret_cast<ID3D12GraphicsCommandList1*>(m_CmdBuffers[index])->ResolveSubresourceRegion(
					ref_cast<Image>(dstImage)->m_Image, dstSubresoucre, static_cast<UINT>(resolveRegion.dstOffset.x), static_cast<UINT>(resolveRegion.dstOffset.y),
					ref_cast<Image>(srcImage)->m_Image, srcSubresoucre, &srcRect, format, d3d12ResolveMode);
			}
		}
		else
		{
			MIRU_FATAL(true, "ERROR: D3D12: Source and Destination arrayLayerCount for resolve image subresources must match.");
		}

		if (useBarrier2)
		{
			b2CI.srcStageMask = base::PipelineStageBit::RESOLVE_BIT;
			b2CI.srcAccess = Barrier::AccessBit::D3D12_RESOLVE_SOURCE;
			b2CI.dstStageMask = base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
			b2CI.dstAccess = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			b2CI.image = srcImage;
			b2CI.oldLayout = Image::Layout::D3D12_RESOLVE_SOURCE;
			b2CI.newLayout = srcImageLayout;
			b2CI.subresourceRange = { resolveRegion.srcSubresource.aspectMask, resolveRegion.srcSubresource.mipLevel, 1, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
			base::Barrier2Ref postResolveBarrierSrc = Barrier2::Create(&b2CI);

			b2CI.srcStageMask = base::PipelineStageBit::RESOLVE_BIT;
			b2CI.srcAccess = Barrier::AccessBit::D3D12_RESOLVE_DEST;
			b2CI.dstStageMask = base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT;
			b2CI.dstAccess = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			b2CI.image = dstImage;
			b2CI.oldLayout = Image::Layout::D3D12_RESOLVE_DEST;
			b2CI.newLayout = dstImageLayout;
			b2CI.subresourceRange = { resolveRegion.dstSubresource.aspectMask, resolveRegion.dstSubresource.mipLevel, 1, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
			base::Barrier2Ref postResolveBarrierDst = Barrier2::Create(&b2CI);

			PipelineBarrier2(index, { base::DependencyBit::NONE_BIT, { postResolveBarrierSrc, postResolveBarrierDst } });
		}
		else
		{
			bCI.image = srcImage;
			bCI.oldLayout = Image::Layout::D3D12_RESOLVE_SOURCE;
			bCI.newLayout = srcImageLayout;
			bCI.subresourceRange = { resolveRegion.srcSubresource.aspectMask, resolveRegion.srcSubresource.mipLevel, 1, resolveRegion.srcSubresource.baseArrayLayer, resolveRegion.srcSubresource.arrayLayerCount };
			base::BarrierRef postResolveBarrierSrc = Barrier::Create(&bCI);

			bCI.image = dstImage;
			bCI.oldLayout = Image::Layout::D3D12_RESOLVE_DEST;
			bCI.newLayout = dstImageLayout;
			bCI.subresourceRange = { resolveRegion.dstSubresource.aspectMask, resolveRegion.dstSubresource.mipLevel, 1, resolveRegion.dstSubresource.baseArrayLayer, resolveRegion.dstSubresource.arrayLayerCount };
			base::BarrierRef postResolveBarrierDst = Barrier::Create(&bCI);

			PipelineBarrier(index, base::PipelineStageBit::TRANSFER_BIT, base::PipelineStageBit::TRANSFER_BIT, base::DependencyBit::NONE_BIT, { postResolveBarrierSrc, postResolveBarrierDst });
		}
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

void CommandBuffer::SetViewport(uint32_t index, const std::vector<base::Viewport>& viewports)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<D3D12_VIEWPORT> d3d12Viewports;
	d3d12Viewports.reserve(viewports.size());
	for (auto& viewport : viewports)
		d3d12Viewports.push_back({ viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth });

	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetViewports(static_cast<UINT>(d3d12Viewports.size()), d3d12Viewports.data());
}

void CommandBuffer::SetScissor(uint32_t index, const std::vector<base::Rect2D>& scissors)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	std::vector<D3D12_RECT> d3d12Scissors;
	d3d12Scissors.reserve(scissors.size());
	for (auto& scissor : scissors)
		d3d12Scissors.push_back({ static_cast<LONG>(scissor.offset.x), static_cast<LONG>(scissor.offset.y), static_cast<LONG>(scissor.extent.width), static_cast<LONG>(scissor.extent.height) });

	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->RSSetScissorRects(static_cast<UINT>(d3d12Scissors.size()), d3d12Scissors.data());
}

void CommandBuffer::ResetQueryPool(uint32_t index, const base::QueryPoolRef& queryPool, uint32_t firstQuery, uint32_t queryCount)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	//No Reset functionality in D3D12.
}

void CommandBuffer::BeginQuery(uint32_t index, const base::QueryPoolRef& queryPool, uint32_t queryIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	QueryPoolRef d3d12QueryPool = ref_cast<QueryPool>(queryPool);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->BeginQuery(d3d12QueryPool->m_QueryHeap, d3d12QueryPool->GetQueryType(), queryIndex);
}

void CommandBuffer::EndQuery(uint32_t index, const base::QueryPoolRef& queryPool, uint32_t queryIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	QueryPoolRef d3d12QueryPool = ref_cast<QueryPool>(queryPool);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->EndQuery(d3d12QueryPool->m_QueryHeap, d3d12QueryPool->GetQueryType(), queryIndex);
}

void CommandBuffer::WriteTimestamp(uint32_t index, const base::QueryPoolRef& queryPool, uint32_t queryIndex, base::PipelineStageBit pipelineStage)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	MIRU_FATAL(!(queryPool->GetCreateInfo().type == QueryPool::Type::TIMESTAMP), "ERROR: D3D12: Can not WriteTimestamp. QueryPool is not of type: TIMESTAMP.");
	EndQuery(index, queryPool, queryIndex);
}

void CommandBuffer::CopyQueryPoolToBuffer(uint32_t index, const base::QueryPoolRef& queryPool, uint32_t firstQuery, uint32_t queryCount, const base::BufferRef& buffer, uint64_t offset, uint64_t stride)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);
	QueryPoolRef d3d12QueryPool = ref_cast<QueryPool>(queryPool);
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ResolveQueryData(
		d3d12QueryPool->m_QueryHeap, d3d12QueryPool->GetQueryType(), firstQuery, queryCount,
		ref_cast<Buffer>(buffer)->m_Buffer, offset);
}

