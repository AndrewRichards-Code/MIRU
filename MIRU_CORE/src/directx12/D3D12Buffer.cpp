#include "common.h"
#include "D3D12Buffer.h"
#include "D3D12Allocator.h"

using namespace miru;
using namespace d3d12;

Buffer::Buffer(Buffer::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		//General Type of Resource
	m_ResourceDesc.Alignment = 0;
	m_ResourceDesc.Width = m_CI.size;								//Alias for bufferSize
	m_ResourceDesc.Height = 1;
	m_ResourceDesc.DepthOrArraySize = 1;
	m_ResourceDesc.MipLevels = 1;
	m_ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;					//Required format for buffers
	m_ResourceDesc.SampleDesc = { 1, 0 };							//Required sampleDesc for buffers
	m_ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			//Required layout for buffers
	m_ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;				//How the resource is to be used
	D3D12_CLEAR_VALUE* clear = nullptr;
	m_CurrentResourceState = ToD3D12BufferType(m_CI.usage);

	D3D12_RESOURCE_ALLOCATION_INFO ai = m_Device->GetResourceAllocationInfo(0, 1, &m_ResourceDesc);

	m_Resource.device = m_Device;
	m_Resource.type = crossplatform::Resource::Type::BUFFER;
	m_Resource.resource = (uint64_t)m_Buffer; // This buffer handle is invalid, it's assigned after the ID3D12Device::CreatePlacedResource()
	m_Resource.usage = static_cast<uint32_t>(m_CI.usage);;
	m_Resource.size = ai.SizeInBytes;
	m_Resource.alignment = ai.Alignment;

	if (m_CI.pMemoryBlock)
	{
		m_CI.pMemoryBlock->AddResource(m_Resource);

		MIRU_ASSERT(m_Device->CreatePlacedResource((ID3D12Heap*)m_Resource.memoryBlock, m_Resource.offset, &m_ResourceDesc, m_CurrentResourceState, clear, IID_PPV_ARGS(&m_Buffer)), "ERROR: D3D12: Failed to place Buffer.");
		D3D12SetName(m_Buffer, m_CI.debugName);

		m_Resource.resource = (uint64_t)m_Buffer;
		m_CI.pMemoryBlock->GetAllocatedResources().at(m_CI.pMemoryBlock.get()).at(m_Resource.id).resource = (uint64_t)m_Buffer;
		m_CI.pMemoryBlock->SubmitData(m_Resource, m_CI.data);
	}
}

Buffer::~Buffer()
{
	SAFE_RELEASE(m_Buffer);
	
	if (m_CI.pMemoryBlock)
		m_CI.pMemoryBlock->RemoveResource(m_Resource.id);
}

D3D12_RESOURCE_STATES Buffer::ToD3D12BufferType(Buffer::UsageBit usage) const
{
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
	m_CI = *pCreateInfo;

	auto resourceDesc = std::dynamic_pointer_cast<Buffer>(m_CI.pBuffer)->m_ResourceDesc;
	auto buffer = std::dynamic_pointer_cast<Buffer>(m_CI.pBuffer)->m_Buffer;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = {0};

	switch (m_CI.type)
	{
		case Type::STORAGE:
		case Type::UNIFORM_TEXEL:
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = resourceDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
			srvDesc.Buffer.FirstElement = m_CI.offset;
			srvDesc.Buffer.NumElements = m_CI.size;
			srvDesc.Buffer.StructureByteStride = m_CI.stride;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			m_Device->CreateShaderResourceView(buffer, &srvDesc, descriptorHandle);
		}
		case Type::STORAGE_TEXEL:
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Format = resourceDesc.Format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = m_CI.offset;
			uavDesc.Buffer.NumElements = m_CI.size;
			uavDesc.Buffer.StructureByteStride = m_CI.stride;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			m_Device->CreateUnorderedAccessView(buffer, nullptr, &uavDesc, descriptorHandle);
		}
		case Type::UNIFORM:
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = buffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = m_CI.size;
			m_Device->CreateConstantBufferView(&cbvDesc, descriptorHandle);
		}
		case Type::INDEX:
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = buffer->GetGPUVirtualAddress();
			ibv.SizeInBytes = m_CI.size;
			ibv.Format = m_CI.stride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		}
		case Type::VERTEX:
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = buffer->GetGPUVirtualAddress();
			vbv.SizeInBytes = m_CI.size;
			vbv.StrideInBytes = m_CI.stride;
		}
		/*case Type::TRANSFORM_FEEDBACK:
		{
			D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
		}*/

	}

}