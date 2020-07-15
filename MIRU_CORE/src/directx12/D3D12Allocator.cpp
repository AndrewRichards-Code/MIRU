#include "miru_core_common.h"
#include "D3D12Allocator.h"
#include "D3D12Context.h"

using namespace miru;
using namespace d3d12;

MemoryBlock::MemoryBlock(MemoryBlock::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Device = reinterpret_cast<ID3D12Device*>(m_CI.pContext->GetDevice());

	m_HeapDesc.SizeInBytes = static_cast<UINT64>(m_CI.blockSize);
	m_HeapDesc.Properties = GetHeapProperties(m_CI.properties);
	m_HeapDesc.Alignment = 0;
	m_HeapDesc.Flags = D3D12_HEAP_FLAG_NONE;

	MIRU_ASSERT(m_Device->CreateHeap(&m_HeapDesc, IID_PPV_ARGS(&m_MemoryHeap)), "ERROR: D3D12: Failed to create Heap.");
	D3D12SetName(m_MemoryHeap, m_CI.debugName);

	s_MemoryBlocks.push_back(this);
	s_AllocatedResources[this];
}

MemoryBlock::~MemoryBlock()
{
	MIRU_CPU_PROFILE_FUNCTION();

	SAFE_RELEASE(m_MemoryHeap);
}

bool MemoryBlock::AddResource(crossplatform::Resource& resource)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_Device != reinterpret_cast<ID3D12Device*>(resource.device))
		return false;

	if (!ResourceBackable(resource))
		return false;

	resource.memoryBlock = (uint64_t)m_MemoryHeap;
	resource.id = GenerateURID();
	s_AllocatedResources[this][resource.id] = resource;
	CalculateOffsets();
	resource = s_AllocatedResources[this][resource.id];

	return true;
}

void MemoryBlock::RemoveResource(uint64_t id)
{
	MIRU_CPU_PROFILE_FUNCTION();

	s_AllocatedResources[this].erase(id);
}

void MemoryBlock::SubmitData(const crossplatform::Resource& resource, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	bool copyByRow = resource.rowPitch && resource.rowPadding && resource.height;

	D3D12_RANGE readRange = { 0, 0}; //We never intend to read from the resource;
	if (m_HeapDesc.Properties.Type == D3D12_HEAP_TYPE_UPLOAD && data)
	{
		ID3D12Resource* d3d12Resource = (ID3D12Resource*)(void*)(resource.resource);
		void* mappedData;
		MIRU_ASSERT(d3d12Resource->Map(0, &readRange, &mappedData), "ERROR: D3D12: Can not map resource.");
		
		if (copyByRow)
		{
			size_t rowWidth = resource.rowPitch - resource.rowPadding;
			std::vector<char> paddingData(resource.rowPadding, 0);
			char* _mappedData = (char*)mappedData;
			char* _data = (char*)data;

			for (size_t i = 0; i < resource.height; i++)
			{
				memcpy(_mappedData, _data, rowWidth);
				_mappedData += rowWidth;
				_data += rowWidth;
				memcpy(_mappedData, paddingData.data(), resource.rowPadding);
				_mappedData += resource.rowPadding;
			}
		}
		else
		{
			memcpy(mappedData, data, size);
		}

		d3d12Resource->Unmap(0, nullptr);
	}
}

void MemoryBlock::AccessData(const crossplatform::Resource& resource, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	D3D12_RANGE readRange = { 0, 0 }; //We never intend to read from the resource;
	if (m_HeapDesc.Properties.Type == D3D12_HEAP_TYPE_UPLOAD && data)
	{
		ID3D12Resource* d3d12Resource = (ID3D12Resource*)(void*)(resource.resource);
		void* mappedData;
		MIRU_ASSERT(d3d12Resource->Map(0, &readRange, &mappedData), "ERROR: D3D12: Can not map resource.");
		memcpy(data, mappedData, size);
		d3d12Resource->Unmap(0, nullptr);
	}
}

D3D12_HEAP_PROPERTIES MemoryBlock::GetHeapProperties(crossplatform::MemoryBlock::PropertiesBit properties)
{
	MIRU_CPU_PROFILE_FUNCTION();

	bool deviceLocal = (m_CI.properties & PropertiesBit::DEVICE_LOCAL_BIT) == PropertiesBit::DEVICE_LOCAL_BIT;
	bool hostVisible = (m_CI.properties & PropertiesBit::HOST_VISIBLE_BIT) == PropertiesBit::HOST_VISIBLE_BIT;
	bool hostCoherent = (m_CI.properties & PropertiesBit::HOST_COHERENT_BIT) == PropertiesBit::HOST_COHERENT_BIT;

	D3D12_HEAP_PROPERTIES result;
	result.Type = deviceLocal ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD;
	result.CPUPageProperty =  D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	result.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	result.CreationNodeMask = 0;
	result.VisibleNodeMask = 0;

	return result;
}