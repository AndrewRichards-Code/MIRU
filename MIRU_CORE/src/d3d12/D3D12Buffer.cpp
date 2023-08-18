#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Buffer.h"
#include "D3D12Allocator.h"

using namespace miru;
using namespace d3d12;

Buffer::Buffer(Buffer::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	
	m_Allocation.width = m_CI.size;
	m_Allocation.height = 0;
	m_Allocation.rowPitch = 0;
	m_Allocation.rowPadding = 0;

	if (arc::BitwiseCheck(m_CI.usage, Buffer::UsageBit::UNIFORM_BIT) || arc::BitwiseCheck(m_CI.usage, Buffer::UsageBit::UNIFORM_TEXEL_BIT))
	{
		m_Allocation.width = arc::Align<size_t>(m_CI.size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	}
	else if (m_CI.imageDimension.width % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
	{
		m_Allocation.rowPitch = arc::Align<size_t>((m_CI.imageDimension.width * m_CI.imageDimension.pixelSize), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		m_Allocation.rowPadding = m_Allocation.rowPitch - (m_CI.imageDimension.width * m_CI.imageDimension.pixelSize);
		m_Allocation.width = m_Allocation.rowPitch * m_CI.imageDimension.height;
		m_Allocation.height = m_CI.imageDimension.height;
	}

	m_ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		//General Type of Resource
	m_ResourceDesc.Alignment = 0;
	m_ResourceDesc.Width = m_Allocation.width;						//Alias for bufferSize
	m_ResourceDesc.Height = 1;
	m_ResourceDesc.DepthOrArraySize = 1;
	m_ResourceDesc.MipLevels = 1;
	m_ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;					//Required format for buffers
	m_ResourceDesc.SampleDesc = { 1, 0 };							//Required sampleDesc for buffers
	m_ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			//Required layout for buffers
	m_ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;				//How the resource is to be used
	D3D12_CLEAR_VALUE* clear = nullptr;

	D3D12_HEAP_TYPE heapType = ref_cast<Allocator>(m_CI.allocator)->GetHeapProperties().Type;
	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
	{
		m_InitialResourceState = ToD3D12BufferType(m_CI.usage);
		m_ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		m_InitialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

	m_D3D12MAllocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
	m_D3D12MAllocationDesc.HeapType = heapType;
	m_D3D12MAllocationDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
	m_D3D12MAllocationDesc.CustomPool = nullptr;

	MIRU_ASSERT(m_CI.allocator->GetD3D12MAAllocator()->CreateResource(&m_D3D12MAllocationDesc, &m_ResourceDesc, m_InitialResourceState, clear, &m_D3D12MAllocation, IID_PPV_ARGS(&m_Buffer)), "ERROR: D3D12: Failed to create Buffer.");
	D3D12SetName(m_Buffer, m_CI.debugName);
	
	m_Allocation.nativeAllocation = (base::NativeAllocation)m_D3D12MAllocation;

	if (m_CI.data)
	{
		m_CI.allocator->SubmitData(m_Allocation, 0, m_CI.size, m_CI.data);
	}
}

Buffer::~Buffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_D3D12MAllocation);
	MIRU_D3D12_SAFE_RELEASE(m_Buffer);
}

D3D12_RESOURCE_STATES Buffer::ToD3D12BufferType(Buffer::UsageBit usage) const
{
	MIRU_CPU_PROFILE_FUNCTION();

	//General cases
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	for (uint32_t i = 0; i < sizeof(uint32_t) * 8; i++)
	{
		Buffer::UsageBit _usage = usage & static_cast<Buffer::UsageBit>(1 << i);

		switch (_usage)
		{
		case Buffer::UsageBit::TRANSFER_SRC_BIT:
			state |= D3D12_RESOURCE_STATE_COPY_SOURCE; break;
		case Buffer::UsageBit::TRANSFER_DST_BIT:
			state |= D3D12_RESOURCE_STATE_COPY_DEST; break;
		case Buffer::UsageBit::UNIFORM_TEXEL_BIT:
			state |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE; break;
		case Buffer::UsageBit::STORAGE_TEXEL_BIT:
			state |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
		case Buffer::UsageBit::UNIFORM_BIT:
			state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
		case Buffer::UsageBit::STORAGE_BIT:
			state |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
		case Buffer::UsageBit::INDEX_BIT:
			state |= D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
		case Buffer::UsageBit::VERTEX_BIT:
			state |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
		case Buffer::UsageBit::INDIRECT_BIT:
			state |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT; break;
		case Buffer::UsageBit::CONDITIONAL_RENDERING_BIT:
			state |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE; break;
		case Buffer::UsageBit::SHADER_BINDING_TABLE_BIT:
			state |= D3D12_RESOURCE_STATE_GENERIC_READ; break;
		case Buffer::UsageBit::TRANSFORM_FEEDBACK_BIT:
			state |= D3D12_RESOURCE_STATE_STREAM_OUT; break;
		case Buffer::UsageBit::TRANSFORM_FEEDBACK_COUNTER_BIT:
			state |= D3D12_RESOURCE_STATE_STREAM_OUT; break;
		case Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT:
			state |= D3D12_RESOURCE_STATE_COMMON; break;
		case Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT:
			state |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE; break;
		case Buffer::UsageBit::ACCELERATION_STRUCTURE_STORAGE_BIT:
			state |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE; break;
		default:
			state |= D3D12_RESOURCE_STATE_COMMON; break;
		}
	}

	//Special cases
	if (state == (D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
		state = D3D12_RESOURCE_STATE_COMMON;
	if (state == (D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_INDEX_BUFFER))
		state = D3D12_RESOURCE_STATE_COMMON;

	return state;
}

BufferView::BufferView(BufferView::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	auto resourceDesc = ref_cast<Buffer>(m_CI.buffer)->m_ResourceDesc;
	auto buffer = ref_cast<Buffer>(m_CI.buffer)->m_Buffer;

	switch (m_CI.type)
	{
		case Type::UNIFORM_TEXEL:
		case Type::UNIFORM:
		{
			m_CBVDesc.BufferLocation = buffer->GetGPUVirtualAddress() + m_CI.offset;
			m_CBVDesc.SizeInBytes = arc::Align<UINT>(static_cast<UINT>(m_CI.size), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			break;
		}
		case Type::STORAGE_TEXEL:
		case Type::STORAGE:
		{
			m_UAVDesc.Format = resourceDesc.Format;
			m_UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			m_UAVDesc.Buffer.FirstElement = m_CI.offset / m_CI.stride;
			m_UAVDesc.Buffer.NumElements = static_cast<UINT>(m_CI.size / m_CI.stride);
			m_UAVDesc.Buffer.StructureByteStride = static_cast<UINT>(m_CI.stride);
			m_UAVDesc.Buffer.CounterOffsetInBytes = 0;
			m_UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			break;
		}
		case Type::INDEX:
		{
			
			m_IBVDesc.BufferLocation = buffer->GetGPUVirtualAddress() + m_CI.offset;
			m_IBVDesc.SizeInBytes = static_cast<UINT>(m_CI.size);
			m_IBVDesc.Format = m_CI.stride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			break;
		}
		case Type::VERTEX:
		{
			m_VBVDesc.BufferLocation = buffer->GetGPUVirtualAddress() + m_CI.offset;
			m_VBVDesc.SizeInBytes = static_cast<UINT>(m_CI.size);
			m_VBVDesc.StrideInBytes = static_cast<UINT>(m_CI.stride);
			break;
		}
		/*case Type::TRANSFORM_FEEDBACK:
		{
			D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
		}*/

	}

}

BufferView::~BufferView()
{
	MIRU_CPU_PROFILE_FUNCTION();
}
#endif