#include "miru_core_common.h"
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

	if ((bool)(m_CI.usage & Buffer::UsageBit::UNIFORM_BIT) || (bool)(m_CI.usage & Buffer::UsageBit::UNIFORM_TEXEL_BIT))
	{
		m_Allocation.width = (m_CI.size + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
	}
	else if (m_CI.imageDimension.width % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
	{
		m_Allocation.rowPitch = ((m_CI.imageDimension.width * m_CI.imageDimension.pixelSize) + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
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

	D3D12_HEAP_TYPE heapType = ref_cast<Allocator>(m_CI.pAllocator)->GetHeapProperties().Type;
	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
		m_InitialResourceState = ToD3D12BufferType(m_CI.usage);
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		m_InitialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

	m_D3D12MAllocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;
	m_D3D12MAllocationDesc.HeapType = heapType;
	m_D3D12MAllocationDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
	m_D3D12MAllocationDesc.CustomPool = nullptr;

	MIRU_ASSERT(m_CI.pAllocator->GetD3D12MAAllocator()->CreateResource(&m_D3D12MAllocationDesc, &m_ResourceDesc, m_InitialResourceState, clear, &m_D3D12MAllocation, IID_PPV_ARGS(&m_Buffer)), "ERROR: D3D12: Failed to create Buffer.");
	D3D12SetName(m_Buffer, m_CI.debugName);
	
	m_Allocation.nativeAllocation = (crossplatform::NativeAllocation)m_D3D12MAllocation;

	if (m_CI.data)
	{
		m_CI.pAllocator->SubmitData(m_Allocation, m_CI.size, m_CI.data);
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

	switch (usage)
	{
	case Buffer::UsageBit::TRANSFER_SRC_BIT:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case Buffer::UsageBit::TRANSFER_DST_BIT:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case Buffer::UsageBit::UNIFORM_TEXEL_BIT:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case Buffer::UsageBit::STORAGE_TEXEL_BIT:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case Buffer::UsageBit::UNIFORM_BIT:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case Buffer::UsageBit::STORAGE_BIT:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case Buffer::UsageBit::INDEX_BIT:
		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case Buffer::UsageBit::VERTEX_BIT:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case Buffer::UsageBit::INDIRECT_BIT:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case Buffer::UsageBit::TRANSFORM_FEEDBACK_BIT:
		return D3D12_RESOURCE_STATE_STREAM_OUT;
	default:
		return D3D12_RESOURCE_STATE_COMMON;
	}
}

BufferView::BufferView(BufferView::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	auto resourceDesc = ref_cast<Buffer>(m_CI.pBuffer)->m_ResourceDesc;
	auto buffer = ref_cast<Buffer>(m_CI.pBuffer)->m_Buffer;

	switch (m_CI.type)
	{
		case Type::UNIFORM_TEXEL:
		case Type::UNIFORM:
		{
			m_CBVDesc.BufferLocation = buffer->GetGPUVirtualAddress();
			m_CBVDesc.SizeInBytes = (static_cast<UINT>(m_CI.size) + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
			break;
		}
		case Type::STORAGE_TEXEL:
		case Type::STORAGE:
		{
			m_UAVDesc.Format = resourceDesc.Format;
			m_UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			m_UAVDesc.Buffer.FirstElement = m_CI.offset;
			m_UAVDesc.Buffer.NumElements = static_cast<UINT>(m_CI.size);
			m_UAVDesc.Buffer.StructureByteStride = static_cast<UINT>(m_CI.stride);
			m_UAVDesc.Buffer.CounterOffsetInBytes = 0;
			m_UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			break;
		}
		case Type::INDEX:
		{
			
			m_IBVDesc.BufferLocation = buffer->GetGPUVirtualAddress();
			m_IBVDesc.SizeInBytes = static_cast<UINT>(m_CI.size);
			m_IBVDesc.Format = m_CI.stride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			break;
		}
		case Type::VERTEX:
		{
			m_VBVDesc.BufferLocation = buffer->GetGPUVirtualAddress();
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