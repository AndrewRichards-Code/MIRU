#include "common.h"
#include "D3D12Allocator.h"
#include "D3D12Context.h"

using namespace miru;
using namespace d3d12;

MemoryBlock::MemoryBlock(MemoryBlock::CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_Device = reinterpret_cast<ID3D12Device*>(m_CI.pContext->GetDevice());
	Ref<d3d12::Context> context = std::dynamic_pointer_cast<d3d12::Context>(m_CI.pContext);

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
	SAFE_RELEASE(m_MemoryHeap);
}

bool MemoryBlock::AddResource(crossplatform::Resource& resource)
{
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
	s_AllocatedResources[this].erase(id);
}

void MemoryBlock::SubmitData(const crossplatform::Resource& resource, void* data)
{
	D3D12_RANGE range = { 0, resource.size};
	if (m_HeapDesc.Properties.CPUPageProperty > D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE)
	{
		ID3D12Resource* d3d12Resource = (ID3D12Resource*)(void*)(resource.resource);
		void* mappedData;
		MIRU_WARN(d3d12Resource->Map(0, &range, &mappedData), "ERROR: D3D12: Can not map resource.");
		
		if (mappedData)
		{
			memcpy(mappedData, data, range.End);
			d3d12Resource->Unmap(0, nullptr);
		}
		else
		{
			d3d12Resource->Unmap(0, nullptr);
			return;
		}
	}
	else
	{
		MIRU_WARN(true, "ERROR: D3D12: Can not submit data. Memory block is not type: D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE or D3D12_CPU_PAGE_PROPERTY_WRITE_BACK.");
	}
}

D3D12_HEAP_PROPERTIES MemoryBlock::GetHeapProperties(crossplatform::MemoryBlock::PropertiesBit properties)
{
	bool deviceLocal = (m_CI.properties & PropertiesBit::DEVICE_LOCAL_BIT) == PropertiesBit::DEVICE_LOCAL_BIT;
	bool hostVisible = (m_CI.properties & PropertiesBit::HOST_VISIBLE_BIT) == PropertiesBit::HOST_VISIBLE_BIT;
	bool hostCoherent = (m_CI.properties & PropertiesBit::HOST_COHERENT_BIT) == PropertiesBit::HOST_COHERENT_BIT;

	D3D12_HEAP_PROPERTIES result;
	result.Type = D3D12_HEAP_TYPE_CUSTOM;
	result.CPUPageProperty = hostVisible ? (/*hostCoherent ? D3D12_CPU_PAGE_PROPERTY_WRITE_BACK :*/ D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE) : D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	result.MemoryPoolPreference = deviceLocal ? D3D12_MEMORY_POOL_L1 : D3D12_MEMORY_POOL_L0;
	result.CreationNodeMask = 0;
	result.VisibleNodeMask = 0;

	return result;
}