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
	MIRU_FATAL(m_Device->CreateFence(m_Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)), "ERROR: D3D12: Failed to a create Fence.");
	D3D12SetName(m_Fence, m_CI.debugName + " : Fence");
	m_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

Fence::~Fence()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_Fence);
}

void Fence::Reset()
{
	MIRU_CPU_PROFILE_FUNCTION();

	Wait();
	m_Value = m_Fence->GetCompletedValue();
	return;
}

bool Fence::GetStatus()
{
	MIRU_CPU_PROFILE_FUNCTION();

	return !(m_Fence->GetCompletedValue() < m_Value);
}

bool Fence::Wait()
{
	MIRU_CPU_PROFILE_FUNCTION();

	UINT64 currentValue = m_Fence->GetCompletedValue();
	if (currentValue < m_Value)
	{
		MIRU_FATAL(m_Fence->SetEventOnCompletion(m_Value, m_Event), "ERROR: D3D12: Failed to wait for Fence.");
		DWORD result = WaitForSingleObject(m_Event, static_cast<DWORD>(m_CI.timeout/1000));
		if (result == WAIT_OBJECT_0)
			return true;
		else if (result == WAIT_TIMEOUT)
			return false;
		else
		{
			MIRU_FATAL(result, "ERROR: D3D12: Failed to wait for Fence.");
			return false;
		}
	}
	return true;
}

void Fence::Signal()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_Value++;
	MIRU_FATAL(m_Fence->Signal(m_Value), "ERROR: D3D12: Failed to a signal Fence.");
}

//Semaphore
Semaphore::Semaphore(Semaphore::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Value = 0;
	MIRU_FATAL(m_Device->CreateFence(m_Value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Semaphore)), "ERROR: D3D12 Failed to create Semaphore.");
	D3D12SetName(m_Semaphore, m_CI.debugName + " : Semaphore");
}

Semaphore::~Semaphore()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_Semaphore);
}

void Semaphore::Signal(uint64_t value)
{
	MIRU_CPU_PROFILE_FUNCTION();
	
	if (m_CI.type == Type::TIMELINE)
	{
		MIRU_FATAL(m_Semaphore->Signal(value), "ERROR: D3D12: Failed to a signal Semaphore.");
	}
	else
	{
		MIRU_FATAL(true, "ERROR: D3D12: Failed to a signal Semaphore, because it's not Type::TIMELINE.")
	}
}

bool Semaphore::Wait(uint64_t value, uint64_t timeout)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_CI.type == Type::TIMELINE)
	{
		UINT64 currentValue = m_Semaphore->GetCompletedValue();
		if (currentValue < value)
		{
			HANDLE _event = CreateEvent(NULL, FALSE, FALSE, NULL);
			MIRU_FATAL(m_Semaphore->SetEventOnCompletion(value, _event), "ERROR: D3D12: Failed to wait for Semaphore.");
			DWORD result = WaitForSingleObject(_event, static_cast<DWORD>(timeout / 1000));
			if (result == WAIT_OBJECT_0)
				return true;
			else if (result == WAIT_TIMEOUT)
				return false;
			else
			{
				MIRU_FATAL(result, "ERROR: D3D12: Failed to wait for Semaphore.");
				return false;
			}
		}
		return true;
	}
	else
	{
		MIRU_FATAL(true, "ERROR: D3D12: Failed to wait for Semaphore, because it's not Type::TIMELINE.")
		return false;
	}
}

uint64_t Semaphore::GetCurrentValue()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_CI.type == Type::TIMELINE)
	{
		return m_Semaphore->GetCompletedValue();
	}
	else
	{
		MIRU_FATAL(true, "ERROR: D3D12: Failed to a get Semaphore's counter value, because it's not Type::TIMELINE.")
		return 0;
	}
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
		barrier.Transition.pResource = ref_cast<Buffer>(m_CI.buffer)->m_Buffer;
		barrier.Transition.StateBefore = ToD3D12ResourceState(m_CI.srcAccess);
		barrier.Transition.StateAfter = ToD3D12ResourceState(m_CI.dstAccess);
		barrier.Transition.Subresource = 0;

		if (barrier.Transition.StateBefore == barrier.Transition.StateAfter) //Check a transition barrier is actaully needed.
			return;
	
		m_Barriers.push_back(barrier);
	}
	else if (m_CI.type == Barrier::Type::IMAGE)
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = ref_cast<Image>(m_CI.image)->m_Image;
		barrier.Transition.StateBefore = Image::ToD3D12ImageLayout(m_CI.oldLayout);
		barrier.Transition.StateAfter = Image::ToD3D12ImageLayout(m_CI.newLayout);

		if (barrier.Transition.StateBefore == barrier.Transition.StateAfter) //Check a transition barrier is actaully needed.
			return;
		if (m_CI.newLayout == Image::Layout::UNKNOWN) //Only transition resource to defined a layout.
			return;

		D3D12_RESOURCE_DESC resDesc = barrier.Transition.pResource->GetDesc();
		if (resDesc.DepthOrArraySize == m_CI.subresourceRange.arrayLayerCount
			&& resDesc.MipLevels == m_CI.subresourceRange.mipLevelCount)
		{
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			m_Barriers.push_back(barrier);
		}
		else
		{
			m_Barriers.reserve(static_cast<size_t>(m_CI.subresourceRange.mipLevelCount) * static_cast<size_t>(m_CI.subresourceRange.arrayLayerCount));
			for (uint32_t i = 0; i < m_CI.subresourceRange.arrayLayerCount; i++)
			{
				for (uint32_t j = 0; j < m_CI.subresourceRange.mipLevelCount; j++)
				{
					barrier.Transition.Subresource = Image::D3D12CalculateSubresource(j + m_CI.subresourceRange.baseMipLevel, i + m_CI.subresourceRange.baseArrayLayer, 0, resDesc.MipLevels, resDesc.DepthOrArraySize);
					m_Barriers.push_back(barrier);
				}
			}
		}
	}
	else if (m_CI.type == Barrier::Type::MEMORY)
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		if (m_CI.buffer)
		{
			barrier.UAV.pResource = ref_cast<Buffer>(m_CI.buffer)->m_Buffer;
			m_Barriers.push_back(barrier);
		}	
		else if (m_CI.image)
		{
			barrier.UAV.pResource = ref_cast<Image>(m_CI.image)->m_Image;
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
		case Barrier::AccessBit::TRANSFORM_FEEDBACK_WRITE_BIT:
		case Barrier::AccessBit::TRANSFORM_FEEDBACK_COUNTER_READ_BIT:
		case Barrier::AccessBit::TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT:
			return D3D12_RESOURCE_STATE_STREAM_OUT;
		case Barrier::AccessBit::CONDITIONAL_RENDERING_READ_BIT:
			return D3D12_RESOURCE_STATE_PREDICATION;
		case Barrier::AccessBit::FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT:
			return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
		case Barrier::AccessBit::ACCELERATION_STRUCTURE_READ_BIT:
		case Barrier::AccessBit::ACCELERATION_STRUCTURE_WRITE_BIT:
			return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		case Barrier::AccessBit::FRAGMENT_DENSITY_MAP_READ_BIT:
			return D3D12_RESOURCE_STATE_COMMON;
		case Barrier::AccessBit::VIDEO_DECODE_READ_BIT:
			return D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
		case Barrier::AccessBit::VIDEO_DECODE_WRITE_BIT:
			return D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
		case Barrier::AccessBit::VIDEO_ENCODE_READ_BIT:
			return D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;
		case Barrier::AccessBit::VIDEO_ENCODE_WRITE_BIT:
			return D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;
		case Barrier::AccessBit::SHADER_BINDING_TABLE_READ_BIT:
			return D3D12_RESOURCE_STATE_COMMON;
		case Barrier::AccessBit::SHADER_SAMPLED_READ_BIT:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case Barrier::AccessBit::SHADER_STORAGE_READ_BIT:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case Barrier::AccessBit::SHADER_STORAGE_WRITE_BIT:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		default:
			return D3D12_RESOURCE_STATE_COMMON | D3D12_RESOURCE_STATE_PRESENT;
	};
}

//Barrier2
Barrier2::Barrier2(Barrier2::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	if (m_CI.type == Barrier::Type::BUFFER)
	{
		D3D12_BUFFER_BARRIER barrier;
		barrier.SyncBefore = ToD3D12BarrierSync(m_CI.srcStageMask);
		barrier.SyncAfter = ToD3D12BarrierSync(m_CI.dstStageMask);
		barrier.AccessBefore = ToD3D12BarrierAccess(m_CI.srcAccess);
		barrier.AccessAfter = ToD3D12BarrierAccess(m_CI.dstAccess);
		barrier.pResource = ref_cast<Buffer>(m_CI.buffer)->m_Buffer;
		barrier.Offset = m_CI.offset;
		barrier.Size = m_CI.size;
		m_BufferBarriers.push_back(barrier);
	}
	else if (m_CI.type == Barrier::Type::IMAGE)
	{
		D3D12_TEXTURE_BARRIER barrier;
		barrier.SyncBefore = ToD3D12BarrierSync(m_CI.srcStageMask);
		barrier.SyncAfter = ToD3D12BarrierSync(m_CI.dstStageMask);
		barrier.AccessBefore = ToD3D12BarrierAccess(m_CI.srcAccess);
		barrier.AccessAfter = ToD3D12BarrierAccess(m_CI.dstAccess);
		barrier.LayoutBefore = ToD3D12BarrierLayout(m_CI.oldLayout);
		barrier.LayoutAfter = ToD3D12BarrierLayout(m_CI.newLayout);
		barrier.pResource = ref_cast<Image>(m_CI.image)->m_Image;
		barrier.Subresources.IndexOrFirstMipLevel = m_CI.subresourceRange.baseMipLevel;
		barrier.Subresources.NumMipLevels = m_CI.subresourceRange.mipLevelCount;
		barrier.Subresources.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
		barrier.Subresources.NumArraySlices = m_CI.subresourceRange.arrayLayerCount;
		barrier.Subresources.FirstPlane = 0;
		barrier.Subresources.NumPlanes = 1;
		barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
		m_TextureBarriers.push_back(barrier);
	}
	else if (m_CI.type == Barrier::Type::MEMORY)
	{
		D3D12_GLOBAL_BARRIER barrier;
		barrier.SyncBefore = ToD3D12BarrierSync(m_CI.srcStageMask);
		barrier.SyncAfter = ToD3D12BarrierSync(m_CI.dstStageMask);
		barrier.AccessBefore = ToD3D12BarrierAccess(m_CI.srcAccess);
		barrier.AccessAfter = ToD3D12BarrierAccess(m_CI.dstAccess);
		m_GlobalBarriers.push_back(barrier);
	}
}

Barrier2::~Barrier2()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

D3D12_BARRIER_SYNC Barrier2::ToD3D12BarrierSync(base::PipelineStageBit pipelineStageBit)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (pipelineStageBit == base::PipelineStageBit::NONE_BIT) //Special early out for NONE_BIT.
		return D3D12_BARRIER_SYNC_NONE;

	D3D12_BARRIER_SYNC result = D3D12_BARRIER_SYNC_NONE;

	std::underlying_type<base::PipelineStageBit>::type mask = 0x1;
	for (size_t i = 0; i < sizeof(mask) * 8; i++)
	{
		auto ToD3D12BarrierSyncSingle = [](base::PipelineStageBit pipelineStageBit) -> D3D12_BARRIER_SYNC
		{
			switch (pipelineStageBit)
			{
			case base::PipelineStageBit::NONE_BIT:
				return D3D12_BARRIER_SYNC_NONE;
			case base::PipelineStageBit::TOP_OF_PIPE_BIT:
				return D3D12_BARRIER_SYNC_ALL;
			case base::PipelineStageBit::DRAW_INDIRECT_BIT:
				return D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
			case base::PipelineStageBit::VERTEX_INPUT_BIT:
				return D3D12_BARRIER_SYNC_INDEX_INPUT;
			case base::PipelineStageBit::VERTEX_SHADER_BIT:
				return D3D12_BARRIER_SYNC_VERTEX_SHADING;
			case base::PipelineStageBit::TESSELLATION_CONTROL_SHADER_BIT:
			case base::PipelineStageBit::TESSELLATION_EVALUATION_SHADER_BIT:
			case base::PipelineStageBit::GEOMETRY_SHADER_BIT:
				return D3D12_BARRIER_SYNC_VERTEX_SHADING;
			case base::PipelineStageBit::FRAGMENT_SHADER_BIT:
				return D3D12_BARRIER_SYNC_PIXEL_SHADING;
			case base::PipelineStageBit::EARLY_FRAGMENT_TESTS_BIT:
			case base::PipelineStageBit::LATE_FRAGMENT_TESTS_BIT:
				return D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			case base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT:
				return D3D12_BARRIER_SYNC_RENDER_TARGET;
			case base::PipelineStageBit::COMPUTE_SHADER_BIT:
				return D3D12_BARRIER_SYNC_COMPUTE_SHADING;
			case base::PipelineStageBit::TRANSFER_BIT:
				return D3D12_BARRIER_SYNC_COPY;
			case base::PipelineStageBit::BOTTOM_OF_PIPE_BIT:
				return  D3D12_BARRIER_SYNC_ALL;
			case base::PipelineStageBit::HOST_BIT:
				return D3D12_BARRIER_SYNC_ALL;
			case base::PipelineStageBit::ALL_GRAPHICS_BIT:
				return D3D12_BARRIER_SYNC_DRAW;
			case base::PipelineStageBit::ALL_COMMANDS_BIT:
				return D3D12_BARRIER_SYNC_ALL;
			case base::PipelineStageBit::TRANSFORM_FEEDBACK_BIT:
				return D3D12_BARRIER_SYNC_VERTEX_SHADING;
			case base::PipelineStageBit::CONDITIONAL_RENDERING_BIT:
				return D3D12_BARRIER_SYNC_PREDICATION;
			case base::PipelineStageBit::FRAGMENT_SHADING_RATE_ATTACHMENT_BIT:
				return D3D12_BARRIER_SYNC_PIXEL_SHADING;
			case base::PipelineStageBit::ACCELERATION_STRUCTURE_BUILD_BIT:
				return D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
			case base::PipelineStageBit::RAY_TRACING_SHADER_BIT:
				return D3D12_BARRIER_SYNC_RAYTRACING;
			case base::PipelineStageBit::FRAGMENT_DENSITY_PROCESS_BIT:
				return D3D12_BARRIER_SYNC_PIXEL_SHADING;
			case base::PipelineStageBit::TASK_SHADER_BIT_EXT:
			case base::PipelineStageBit::MESH_SHADER_BIT_EXT:
				return D3D12_BARRIER_SYNC_VERTEX_SHADING;
			case base::PipelineStageBit::VIDEO_DECODE_BIT:
				return D3D12_BARRIER_SYNC_VIDEO_DECODE;
			case base::PipelineStageBit::VIDEO_ENCODE_BIT:
				return D3D12_BARRIER_SYNC_VIDEO_ENCODE;
			case base::PipelineStageBit::COPY_BIT:
				return D3D12_BARRIER_SYNC_COPY;
			case base::PipelineStageBit::RESOLVE_BIT:
				return D3D12_BARRIER_SYNC_RESOLVE;
			case base::PipelineStageBit::BLIT_BIT:
				return D3D12_BARRIER_SYNC_ALL;
			case base::PipelineStageBit::CLEAR_BIT:
				return D3D12_BARRIER_SYNC_ALL;
			case base::PipelineStageBit::INDEX_INPUT_BIT:
				return D3D12_BARRIER_SYNC_INDEX_INPUT;
			case base::PipelineStageBit::VERTEX_ATTRIBUTE_INPUT_BIT:
				return D3D12_BARRIER_SYNC_VERTEX_SHADING;
			case base::PipelineStageBit::PRE_RASTERIZATION_SHADERS_BIT:
				return D3D12_BARRIER_SYNC_VERTEX_SHADING;
			default:
				return D3D12_BARRIER_SYNC_NONE;
			}
		};

		result |= ToD3D12BarrierSyncSingle(base::PipelineStageBit(mask << i) & pipelineStageBit);
	}

	return result;
}

D3D12_BARRIER_ACCESS Barrier2::ToD3D12BarrierAccess(Barrier::AccessBit access)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (access == Barrier::AccessBit::NONE_BIT) //Special early out for NONE_BIT.
		return D3D12_BARRIER_ACCESS_NO_ACCESS;

	D3D12_BARRIER_ACCESS result = D3D12_BARRIER_ACCESS_COMMON;

	std::underlying_type<Barrier::AccessBit>::type mask = 0x1;
	for (size_t i = 0; i < sizeof(mask) * 8; i++)
	{
		auto ToD3D12BarrierAccessSingle = [](Barrier::AccessBit access) -> D3D12_BARRIER_ACCESS
		{
			switch (access)
			{
			case Barrier::AccessBit::NONE_BIT:
				return D3D12_BARRIER_ACCESS_COMMON;
			case Barrier::AccessBit::INDIRECT_COMMAND_READ_BIT:
				return D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
			case Barrier::AccessBit::INDEX_READ_BIT:
				return D3D12_BARRIER_ACCESS_INDEX_BUFFER;
			case Barrier::AccessBit::VERTEX_ATTRIBUTE_READ_BIT:
				return D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
			case Barrier::AccessBit::UNIFORM_READ_BIT:
				return D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
			case Barrier::AccessBit::INPUT_ATTACHMENT_READ_BIT:
				return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			case Barrier::AccessBit::SHADER_READ_BIT:
				return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			case Barrier::AccessBit::SHADER_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			case Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT:
			case Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_RENDER_TARGET;
			case Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_READ_BIT:
				return D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
			case Barrier::AccessBit::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
			case Barrier::AccessBit::TRANSFER_READ_BIT:
			case Barrier::AccessBit::HOST_READ_BIT:
			case Barrier::AccessBit::MEMORY_READ_BIT:
				return D3D12_BARRIER_ACCESS_COPY_DEST;
			case Barrier::AccessBit::TRANSFER_WRITE_BIT:
			case Barrier::AccessBit::HOST_WRITE_BIT:
			case Barrier::AccessBit::MEMORY_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_COPY_SOURCE;
			case Barrier::AccessBit::TRANSFORM_FEEDBACK_WRITE_BIT:
			case Barrier::AccessBit::TRANSFORM_FEEDBACK_COUNTER_READ_BIT:
			case Barrier::AccessBit::TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_STREAM_OUTPUT;
			case Barrier::AccessBit::CONDITIONAL_RENDERING_READ_BIT:
				return D3D12_BARRIER_ACCESS_PREDICATION;
			case Barrier::AccessBit::FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT:
				return D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE;
			case Barrier::AccessBit::ACCELERATION_STRUCTURE_READ_BIT:
				return D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
			case Barrier::AccessBit::ACCELERATION_STRUCTURE_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
			case Barrier::AccessBit::FRAGMENT_DENSITY_MAP_READ_BIT:
				return D3D12_BARRIER_ACCESS_COMMON;
			case Barrier::AccessBit::VIDEO_DECODE_READ_BIT:
				return D3D12_BARRIER_ACCESS_VIDEO_DECODE_READ;
			case Barrier::AccessBit::VIDEO_DECODE_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_VIDEO_DECODE_WRITE;
			case Barrier::AccessBit::VIDEO_ENCODE_READ_BIT:
				return D3D12_BARRIER_ACCESS_VIDEO_ENCODE_READ;
			case Barrier::AccessBit::VIDEO_ENCODE_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_VIDEO_ENCODE_WRITE;
			case Barrier::AccessBit::SHADER_BINDING_TABLE_READ_BIT:
				return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			case Barrier::AccessBit::SHADER_SAMPLED_READ_BIT:
				return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			case Barrier::AccessBit::SHADER_STORAGE_READ_BIT:
				return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			case Barrier::AccessBit::SHADER_STORAGE_WRITE_BIT:
				return D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			case Barrier::AccessBit::D3D12_RESOLVE_SOURCE:
				return D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
			case Barrier::AccessBit::D3D12_RESOLVE_DEST:
				return D3D12_BARRIER_ACCESS_RESOLVE_DEST;
			default:
				return D3D12_BARRIER_ACCESS_COMMON;
			}
		};
		
		result |= ToD3D12BarrierAccessSingle(Barrier::AccessBit(mask << i) & access);
	}

	return result;
}

D3D12_BARRIER_LAYOUT Barrier2::ToD3D12BarrierLayout(Image::Layout layout)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (layout)
	{
	case Image::Layout::UNKNOWN:
		return D3D12_BARRIER_LAYOUT_UNDEFINED;
	case Image::Layout::GENERAL:
		return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
	case Image::Layout::COLOUR_ATTACHMENT_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
	case Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
	case Image::Layout::DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
	case Image::Layout::SHADER_READ_ONLY_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
	case Image::Layout::TRANSFER_SRC_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
	case Image::Layout::TRANSFER_DST_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_COPY_DEST;
	case Image::Layout::PREINITIALISED:
		return D3D12_BARRIER_LAYOUT_COMMON;
	case Image::Layout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
	case Image::Layout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
	case Image::Layout::DEPTH_ATTACHMENT_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
	case Image::Layout::DEPTH_READ_ONLY_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
	case Image::Layout::STENCIL_ATTACHMENT_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
	case Image::Layout::STENCIL_READ_ONLY_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
	case Image::Layout::PRESENT_SRC:
	case Image::Layout::SHARED_PRESENT:
		return D3D12_BARRIER_LAYOUT_PRESENT;
	case Image::Layout::D3D12_RESOLVE_SOURCE:
		return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
	case Image::Layout::D3D12_RESOLVE_DEST:
		return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
	case Image::Layout::D3D12_UNORDERED_ACCESS:
		return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
	case Image::Layout::D3D12_NON_PIXEL_SHADER_READ_ONLY_OPTIMAL:
		return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
	default:
		return D3D12_BARRIER_LAYOUT_COMMON;
	}
}