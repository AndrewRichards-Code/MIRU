#include "miru_core_common.h"
#include "D3D12Sync.h"

#include "D3D12Buffer.h"
#include "D3D12Image.h"

using namespace miru;
using namespace d3d12;

//Fence
Fence::Fence(Fence::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Value = 0;
	MIRU_ASSERT(m_Device->CreateFence(m_Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)), "ERROR: D3D12: Failed to a create Fence.");
	m_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

Fence::~Fence()
{
	MIRU_CPU_PROFILE_FUNCTION();

	SAFE_RELEASE(m_Fence);
}

void Fence::Reset()
{
	MIRU_CPU_PROFILE_FUNCTION();

	return;
}

bool Fence::GetStatus()
{
	MIRU_CPU_PROFILE_FUNCTION();

	return Wait();
}

bool Fence::Wait()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_Fence->GetCompletedValue() == m_Value)
	{
		MIRU_ASSERT(m_Fence->SetEventOnCompletion(m_Value, m_Event), "ERROR: D3D12: Failed to wait for Fence.");
		WaitForSingleObject(m_Event, static_cast<DWORD>(m_CI.timeout/1000));
		return false;
	}
	else
		return true;
}

void Fence::Signal()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_Value++;
	MIRU_ASSERT(m_Fence->Signal(m_Value), "ERROR: D3D12: Failed to a signal Fence.");
}

//Semaphore
Semaphore::Semaphore(Semaphore::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Value = 0;
	MIRU_ASSERT(m_Device->CreateFence(m_Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Semaphore)), "ERROR: D3D12 Failed to create Semaphore.");
}

Semaphore::~Semaphore()
{
	MIRU_CPU_PROFILE_FUNCTION();

	SAFE_RELEASE(m_Semaphore);
}

//Event - Split Barrier
Event::Event(Event::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();
}

Event::~Event()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void Event::Set()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void Event::Reset()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

bool Event::GetStatus()
{
	MIRU_CPU_PROFILE_FUNCTION();

	return false;
}

//Barrier
Barrier::Barrier(Barrier::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_Barriers.reserve(1);
	if (m_CI.type == Barrier::Type::BUFFER)
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = ref_cast<Buffer>(m_CI.pBuffer)->m_Buffer;
		barrier.Transition.StateBefore = ToD3D12ResourceState(m_CI.srcAccess);
		barrier.Transition.StateAfter = ToD3D12ResourceState(m_CI.dstAccess);
		barrier.Transition.Subresource = 0;

		if (ref_cast<Buffer>(m_CI.pBuffer)->m_CurrentResourceState != ToD3D12ResourceState(m_CI.srcAccess))
			barrier.Transition.StateBefore = ref_cast<Buffer>(m_CI.pBuffer)->m_CurrentResourceState;
		
		if (barrier.Transition.StateBefore == barrier.Transition.StateAfter) //Check a transition barrier is actaully needed.
			return;
	
		m_Barriers.push_back(barrier);
		ref_cast<Buffer>(m_CI.pBuffer)->m_CurrentResourceState = barrier.Transition.StateAfter;
	}
	else if (m_CI.type == Barrier::Type::IMAGE)
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = ref_cast<Image>(m_CI.pImage)->m_Image;
		barrier.Transition.StateBefore = Image::ToD3D12ImageLayout(m_CI.oldLayout);
		barrier.Transition.StateAfter = Image::ToD3D12ImageLayout(m_CI.newLayout);

		if(m_CI.newLayout == Image::Layout::GENERAL && m_CI.dstAccess == Barrier::AccessBit::SHADER_WRITE_BIT) //Vulkan representation a UAV.
			barrier.Transition.StateAfter = ToD3D12ResourceState(m_CI.dstAccess);
		
		if (ref_cast<Image>(m_CI.pImage)->m_CurrentResourceState != Image::ToD3D12ImageLayout(m_CI.oldLayout))
			barrier.Transition.StateBefore = ref_cast<Image>(m_CI.pImage)->m_CurrentResourceState;

		if (barrier.Transition.StateBefore == barrier.Transition.StateAfter) //Check a transition barrier is actaully needed.
			return;

		D3D12_RESOURCE_DESC resDesc = barrier.Transition.pResource->GetDesc();
		if (resDesc.DepthOrArraySize == m_CI.subresoureRange.arrayLayerCount
			&& resDesc.MipLevels == m_CI.subresoureRange.mipLevelCount)
		{
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			m_Barriers.push_back(barrier);
		}
		else
		{
			m_Barriers.reserve(static_cast<size_t>(m_CI.subresoureRange.mipLevelCount - m_CI.subresoureRange.baseMipLevel) 
				* static_cast<size_t>(m_CI.subresoureRange.arrayLayerCount - m_CI.subresoureRange.baseArrayLayer));
			for (uint32_t i = m_CI.subresoureRange.baseArrayLayer; i < m_CI.subresoureRange.arrayLayerCount; i++)
			{
				for (uint32_t j = m_CI.subresoureRange.baseMipLevel; j < m_CI.subresoureRange.mipLevelCount; j++)
				{
					barrier.Transition.Subresource = Image::D3D12CalculateSubresource(j, i, 0, m_CI.subresoureRange.mipLevelCount, m_CI.subresoureRange.arrayLayerCount);
					m_Barriers.push_back(barrier);
				}
			}
		}
		ref_cast<Image>(m_CI.pImage)->m_CurrentResourceState = barrier.Transition.StateAfter;
	}
	else if (m_CI.type == Barrier::Type::MEMORY)
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		if (m_CI.pBuffer)
		{
			barrier.UAV.pResource = ref_cast<Buffer>(m_CI.pBuffer)->m_Buffer;
			m_Barriers.push_back(barrier);
		}	
		else if (m_CI.pImage)
		{
			barrier.UAV.pResource = ref_cast<Image>(m_CI.pImage)->m_Image;
			m_Barriers.push_back(barrier);
		}
		else
			m_Barriers[0] = D3D12_RESOURCE_BARRIER();
	}
	else
		m_Barriers[0] = D3D12_RESOURCE_BARRIER();

}

Barrier::~Barrier()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

D3D12_RESOURCE_STATES Barrier::ToD3D12ResourceState(Barrier::AccessBit access)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (access)
	{
		case Barrier::AccessBit::INDIRECT_COMMAND_READ_BIT:
			return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		case Barrier::AccessBit::INDEX_READ_BIT:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case Barrier::AccessBit::VERTEX_ATTRIBUTE_READ_BIT:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case Barrier::AccessBit::UNIFORM_READ_BIT:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case Barrier::AccessBit::INPUT_ATTACHMENT_READ_BIT:
			return D3D12_RESOURCE_STATE_COMMON;
		case Barrier::AccessBit::SHADER_READ_BIT:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case Barrier::AccessBit::SHADER_WRITE_BIT:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_READ_BIT:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case Barrier::AccessBit::TRANSFER_READ_BIT:
		case Barrier::AccessBit::HOST_READ_BIT:
		case Barrier::AccessBit::MEMORY_READ_BIT:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case Barrier::AccessBit::TRANSFER_WRITE_BIT:
		case Barrier::AccessBit::HOST_WRITE_BIT:
		case Barrier::AccessBit::MEMORY_WRITE_BIT:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case Barrier::AccessBit::TRANSFORM_FEEDBACK_WRITE_BIT_EXT:
		case Barrier::AccessBit::TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT:
		case Barrier::AccessBit::TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:
			return D3D12_RESOURCE_STATE_STREAM_OUT;
		default:
			return D3D12_RESOURCE_STATE_COMMON | D3D12_RESOURCE_STATE_PRESENT;
	};
}
