#include "common.h"
#include "D3D12CommandPoolBuffer.h"
#include "D3D12Context.h"
#include "D3D12Swapchain.h"
#include "D3D12Sync.h"
#include "D3D12Image.h"

using namespace miru;
using namespace d3d12;

//CmdPool
CommandPool::CommandPool(CommandPool::CreateInfo* pCreateInfo)
	:m_Device(std::dynamic_pointer_cast<Context>(pCreateInfo->pContext)->m_Device),
	m_Queue(std::dynamic_pointer_cast<Context>(pCreateInfo->pContext)->m_Queues[pCreateInfo->queueFamilyIndex])
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
	m_Device = std::dynamic_pointer_cast<CommandPool>(m_CI.pCommandPool)->m_Device;
	m_CmdPool = std::dynamic_pointer_cast<CommandPool>(m_CI.pCommandPool)->m_CmdPool;
	D3D12_COMMAND_QUEUE_DESC queueDesc = std::dynamic_pointer_cast<CommandPool>(m_CI.pCommandPool)->m_Queue->GetDesc();

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
			reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ExecuteBundle(reinterpret_cast<ID3D12GraphicsCommandList*>(std::dynamic_pointer_cast<CommandBuffer>(commandBuffer)->m_CmdBuffers[secondaryIndex]));
	}

}

void CommandBuffer::Submit(const std::vector<uint32_t>& cmdBufferIndices, std::vector<Ref<crossplatform::Semaphore>>& waits, std::vector<Ref<crossplatform::Semaphore>>& signals, crossplatform::PipelineStageBit pipelineStage, Ref<crossplatform::Fence> fence)
{
	ID3D12CommandQueue* queue = std::dynamic_pointer_cast<CommandPool>(m_CI.pCommandPool)->m_Queue;
	std::vector<ID3D12CommandList*>submitCmdBuffers;
	for (auto& index : cmdBufferIndices)
	{
		if (index < m_CI.commandBufferCount)
			submitCmdBuffers.push_back(m_CmdBuffers[index]);
	}
	for (auto& wait : waits)
		queue->Wait(std::dynamic_pointer_cast<Semaphore>(wait)->m_Semaphore, std::dynamic_pointer_cast<Semaphore>(wait)->GetValue());
	
	queue->ExecuteCommandLists(static_cast<uint32_t>(submitCmdBuffers.size()), submitCmdBuffers.data());
	
	for (auto& signal : signals)
	{
		std::dynamic_pointer_cast<Semaphore>(signal)->GetValue()++;
		queue->Signal(std::dynamic_pointer_cast<Semaphore>(signal)->m_Semaphore, std::dynamic_pointer_cast<Semaphore>(signal)->GetValue());
	}
}

void CommandBuffer::Present(const std::vector<uint32_t>& cmdBufferIndices, Ref<crossplatform::Swapchain> swapchain, std::vector<Ref<crossplatform::Fence>>& draws, std::vector<Ref<crossplatform::Semaphore>>& acquires, std::vector<Ref<crossplatform::Semaphore>>& submits)
{
	size_t swapchainImageCount = std::dynamic_pointer_cast<Swapchain>(swapchain)->m_SwapchainRTVs.size();

	if (swapchainImageCount != cmdBufferIndices.size()
		|| swapchainImageCount != draws.size()
		|| swapchainImageCount != acquires.size()
		|| swapchainImageCount != submits.size())
	{
		MIRU_ASSERT(true, "ERROR: D3D12: SwapchainImageCount and number of synchronisation objects does not match.");
	}
	
	IDXGISwapChain4* d3d12Swapchain = std::dynamic_pointer_cast<Swapchain>(swapchain)->m_Swapchain;
	ID3D12CommandQueue* d3d12Queue = std::dynamic_pointer_cast<CommandPool>(m_CI.pCommandPool)->m_Queue;

	draws[m_CurrentFrame]->Wait();
	draws[m_CurrentFrame]->Reset();

	std::vector<Ref<crossplatform::Semaphore>> blank = {};
	UINT imageIndex = d3d12Swapchain->GetCurrentBackBufferIndex();
	Submit({cmdBufferIndices[imageIndex]}, blank, blank, crossplatform::PipelineStageBit::NONE, {});
	
	MIRU_ASSERT(d3d12Swapchain->Present(1, 0), "ERROR: D3D12: Failed to present the Image from Swapchain.");
	std::dynamic_pointer_cast<Fence>(draws[m_CurrentFrame])->GetValue()++;
	d3d12Queue->Signal(std::dynamic_pointer_cast<Fence>(draws[m_CurrentFrame])->m_Fence, std::dynamic_pointer_cast<Fence>(draws[m_CurrentFrame])->GetValue());

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
		for (auto& _barrier : std::dynamic_pointer_cast<Barrier>(barrier)->m_Barriers)
			_barriers.push_back(_barrier);

	}
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

	D3D12_RESOURCE_DESC resourceDesc = std::dynamic_pointer_cast<Image>(image)->m_ResourceDesc;
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (size_t h = 0; h < subresourceRanges.size(); h++)
	{
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].mipLevelCount; i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = resourceDesc.Format;
			switch (resourceDesc.Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_UNKNOWN:
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_UNKNOWN;
				break;
			}
			case D3D12_RESOURCE_DIMENSION_BUFFER:
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
				rtvDesc.Buffer.FirstElement = 0;
				rtvDesc.Buffer.NumElements = static_cast<UINT>(resourceDesc.Width);
				break;
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
					rtvDesc.Texture1DArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
					rtvDesc.Texture1DArray.ArraySize = subresourceRanges[h].arrayLayerCount;
					rtvDesc.Texture1DArray.MipSlice = i;
					break;
				}
				else
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
					rtvDesc.Texture1D.MipSlice = i;
					break;
				}
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
						rtvDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						rtvDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						break;
					}
					else
					{
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						rtvDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						rtvDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						rtvDesc.Texture2D.MipSlice = i;
						rtvDesc.Texture2D.PlaneSlice = 0;
						break;
					}
				}
				else
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
						break;
					}
					else
					{
						rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						rtvDesc.Texture2D.MipSlice = i;
						rtvDesc.Texture2D.PlaneSlice = 0;
						break;
					}
				}
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
				rtvDesc.Texture3D.FirstWSlice = subresourceRanges[h].baseArrayLayer;
				rtvDesc.Texture3D.WSize = subresourceRanges[h].arrayLayerCount;
				rtvDesc.Texture3D.MipSlice = i;
				break;
			}
			}
			m_Device->CreateRenderTargetView(std::dynamic_pointer_cast<Image>(image)->m_Image, 0/*&rtvDesc*/, handle);
			handle.ptr += rtvDescriptorSize;
		}
	}

	handle = heap->GetCPUDescriptorHandleForHeapStart();
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearRenderTargetView(handle, clear.float32, 0, nullptr);
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

	D3D12_RESOURCE_DESC resourceDesc = std::dynamic_pointer_cast<Image>(image)->m_ResourceDesc;
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	for (size_t h = 0; h < subresourceRanges.size(); h++)
	{
		for (uint32_t i = subresourceRanges[h].baseMipLevel; i < subresourceRanges[h].mipLevelCount; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Format = resourceDesc.Format;
			switch (resourceDesc.Dimension)
			{
			case D3D12_RESOURCE_DIMENSION_BUFFER:
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			case D3D12_RESOURCE_DIMENSION_UNKNOWN:
			{
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_UNKNOWN;
				break;
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
					dsvDesc.Texture1DArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
					dsvDesc.Texture1DArray.ArraySize = subresourceRanges[h].arrayLayerCount;
					dsvDesc.Texture1DArray.MipSlice = i;
					break;
				}
				else
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
					dsvDesc.Texture1D.MipSlice = i;
					break;
				}
			}
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			{
				if (resourceDesc.DepthOrArraySize > 1)
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
						dsvDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						dsvDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						break;
					}
					else
					{
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
						dsvDesc.Texture2DMSArray.FirstArraySlice = subresourceRanges[h].baseArrayLayer;
						dsvDesc.Texture2DMSArray.ArraySize = subresourceRanges[h].arrayLayerCount;
						dsvDesc.Texture2D.MipSlice = i;
						break;
					}
				}
				else
				{
					if (resourceDesc.SampleDesc.Count > 1)
					{
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
						break;
					}
					else
					{
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
						dsvDesc.Texture2D.MipSlice = i;
						break;
					}
				}
			}
			}
			m_Device->CreateDepthStencilView(std::dynamic_pointer_cast<Image>(image)->m_Image, 0/*&dsvDesc*/, handle);
			handle.ptr += rtvDescriptorSize;
			descriptorCount++;
		}
	}

	handle = heap->GetCPUDescriptorHandleForHeapStart();
	reinterpret_cast<ID3D12GraphicsCommandList*>(m_CmdBuffers[index])->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear.depth, (clear.stencil * UINT8_MAX/UINT32_MAX), 0, nullptr);
	SAFE_RELEASE(heap);
}