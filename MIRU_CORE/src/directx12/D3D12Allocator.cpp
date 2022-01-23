#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Allocator.h"
#include "D3D12Context.h"

using namespace miru;
using namespace d3d12;

Allocator::Allocator(Allocator::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	m_Device = reinterpret_cast<ID3D12Device*>(m_CI.pContext->GetDevice());


	m_AllocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
	m_AllocatorDesc.pDevice = m_Device;
	m_AllocatorDesc.PreferredBlockSize = static_cast<UINT64>(m_CI.blockSize);
	m_AllocatorDesc.pAllocationCallbacks = nullptr;
	m_AllocatorDesc.pAdapter = dynamic_cast<IDXGIAdapter*>(ref_cast<Context>(m_CI.pContext)->m_PhysicalDevices.m_PDIs[0].m_Adapter);

	MIRU_ASSERT(D3D12MA::CreateAllocator(&m_AllocatorDesc, &m_Allocator), "ERROR: D3D12: Failed to create Allocator.");
	//D3D12SetName(m_MemoryHeap, m_CI.debugName);
}

Allocator::~Allocator()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_Allocator);
}

void* Allocator::GetNativeAllocator()
{
	return reinterpret_cast<void*>(m_Allocator);
}

void Allocator::SubmitData(const crossplatform::Allocation& allocation, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();


	if (allocation.nativeAllocation && size > 0  && data)
	{
		ID3D12Resource* d3d12Resource = allocation.GetD3D12MAAllocaton()->GetResource();

		bool uploadHeap = GetHeapProperties().Type == D3D12_HEAP_TYPE_UPLOAD;
		if (uploadHeap)
		{
			void* mappedData;
			D3D12_RANGE readRange = { 0, 0 }; //We never intend to read from the resource;
			MIRU_ASSERT(d3d12Resource->Map(0, &readRange, &mappedData), "ERROR: D3D12: Can not map resource.");

			bool copyByRow = allocation.rowPitch && allocation.rowPadding && allocation.height;
			if (copyByRow)
			{
				size_t rowWidth = allocation.rowPitch - allocation.rowPadding;
				std::vector<char> paddingData(allocation.rowPadding, 0);
				char* _mappedData = (char*)mappedData;
				char* _data = (char*)data;

				for (size_t i = 0; i < allocation.height; i++)
				{
					memcpy(_mappedData, _data, rowWidth);
					_mappedData += rowWidth;
					_data += rowWidth;
					memcpy(_mappedData, paddingData.data(), allocation.rowPadding);
					_mappedData += allocation.rowPadding;
				}
			}
			else
			{
				memcpy(mappedData, data, size);
			}

			d3d12Resource->Unmap(0, nullptr);
		}
	}
}

void Allocator::AccessData(const crossplatform::Allocation& allocation, size_t size, void* data)
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (allocation.nativeAllocation && size > 0 && data)
	{
		ID3D12Resource* d3d12Resource = allocation.GetD3D12MAAllocaton()->GetResource();

		bool uploadHeap = GetHeapProperties().Type == D3D12_HEAP_TYPE_UPLOAD;
		if (uploadHeap)
		{
			void* mappedData;
			D3D12_RANGE readRange = { 0, 0 }; //We never intend to read from the resource;
			MIRU_ASSERT(d3d12Resource->Map(0, &readRange, &mappedData), "ERROR: D3D12: Can not map resource.");
			memcpy(data, mappedData, size);
			d3d12Resource->Unmap(0, nullptr);
		}
	}
}

D3D12_HEAP_PROPERTIES Allocator::GetHeapProperties()
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
#endif