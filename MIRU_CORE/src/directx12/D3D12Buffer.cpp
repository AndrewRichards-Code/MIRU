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

	m_ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		//General Type of Resource
	m_ResourceDesc.Alignment = 0;
	m_ResourceDesc.Width = ((bool)(m_CI.usage & Buffer::UsageBit::UNIFORM) || (bool)(m_CI.usage & Buffer::UsageBit::UNIFORM_TEXEL)) ? max(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,  m_CI.size) : m_CI.size;								//Alias for bufferSize
	m_ResourceDesc.Height = 1;
	m_ResourceDesc.DepthOrArraySize = 1;
	m_ResourceDesc.MipLevels = 1;
	m_ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;					//Required format for buffers
	m_ResourceDesc.SampleDesc = { 1, 0 };							//Required sampleDesc for buffers
	m_ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			//Required layout for buffers
	m_ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;				//How the resource is to be used
	D3D12_CLEAR_VALUE* clear = nullptr;

	D3D12_HEAP_TYPE heapType = ref_cast<MemoryBlock>(m_CI.pMemoryBlock)->m_HeapDesc.Properties.Type;
	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
		m_CurrentResourceState = ToD3D12BufferType(m_CI.usage);
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		m_CurrentResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

	m_AllocationInfo = m_Device->GetResourceAllocationInfo(0, 1, &m_ResourceDesc);

	m_Resource.device = m_Device;
	m_Resource.type = crossplatform::Resource::Type::BUFFER;
	m_Resource.resource = (uint64_t)m_Buffer; // This buffer handle is invalid, it's assigned after the ID3D12Device::CreatePlacedResource()
	m_Resource.usage = static_cast<uint32_t>(m_CI.usage);
	m_Resource.size = m_AllocationInfo.SizeInBytes;
	m_Resource.alignment = m_AllocationInfo.Alignment;

	if (m_CI.pMemoryBlock)
	{
		m_CI.pMemoryBlock->AddResource(m_Resource);

		MIRU_ASSERT(m_Device->CreatePlacedResource((ID3D12Heap*)m_Resource.memoryBlock, m_Resource.offset, &m_ResourceDesc, m_CurrentResourceState, clear, IID_PPV_ARGS(&m_Buffer)), "ERROR: D3D12: Failed to place Buffer.");
		D3D12SetName(m_Buffer, m_CI.debugName);

		m_Resource.resource = (uint64_t)m_Buffer;
		m_CI.pMemoryBlock->GetAllocatedResources().at(m_CI.pMemoryBlock.get()).at(m_Resource.id).resource = (uint64_t)m_Buffer;
		m_CI.pMemoryBlock->SubmitData(m_Resource, m_CI.size, m_CI.data);
	}
}

Buffer::~Buffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	SAFE_RELEASE(m_Buffer);
	
	if (m_CI.pMemoryBlock)
		m_CI.pMemoryBlock->RemoveResource(m_Resource.id);
}

D3D12_RESOURCE_STATES Buffer::ToD3D12BufferType(Buffer::UsageBit usage) const
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (usage)
	{
	case Buffer::UsageBit::TRANSFER_SRC:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case Buffer::UsageBit::TRANSFER_DST:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case Buffer::UsageBit::UNIFORM_TEXEL:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case Buffer::UsageBit::STORAGE_TEXEL:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case Buffer::UsageBit::UNIFORM:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case Buffer::UsageBit::STORAGE:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case Buffer::UsageBit::INDEX:
		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case Buffer::UsageBit::VERTEX:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case Buffer::UsageBit::INDIRECT:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case Buffer::UsageBit::TRANSFORM_FEEDBACK:
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
			m_CBVDesc.SizeInBytes = (static_cast<UINT>(m_CI.size) + 255) & ~255;
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