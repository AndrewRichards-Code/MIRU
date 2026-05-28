#include "D3D12Query.h"
#include "D3D12Device.h"

using namespace miru;
using namespace d3d12;

QueryPool::QueryPool(CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Device>(pCreateInfo->device)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	D3D12_QUERY_HEAP_TYPE type = D3D12_QUERY_HEAP_TYPE(0);
	switch (m_CI.type)
	{
	case Type::OCCLUSION:
		type = D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		break;
	case Type::PIPELINE_STATISTICS:
		type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
		break;
	case Type::TIMESTAMP:
		type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		break;
	default:
		break;
	}

	m_QueryHeapDesc.Type = type;
	m_QueryHeapDesc.Count = m_CI.count;
	m_QueryHeapDesc.NodeMask = 0;

	MIRU_FATAL(m_Device->CreateQueryHeap(&m_QueryHeapDesc, IID_PPV_ARGS(&m_QueryHeap)), "ERROR: D3D12: Failed to create QueryHeap.");
	D3D12SetName(m_QueryHeap, m_CI.debugName);

	CreateReadbackBuffer();
}

QueryPool::~QueryPool()
{
	MIRU_CPU_PROFILE_FUNCTION();
	
	MIRU_D3D12_SAFE_RELEASE(m_QueryHeap);
}

void QueryPool::Reset(uint32_t firstQuery, uint32_t queryCount)
{
	MIRU_CPU_PROFILE_FUNCTION();

	//No Reset functionality in D3D12.
}

D3D12_QUERY_TYPE QueryPool::GetQueryType()
{
	D3D12_QUERY_TYPE type = D3D12_QUERY_TYPE(0);
	switch (m_CI.type)
	{
	case Type::OCCLUSION:
		type = D3D12_QUERY_TYPE_OCCLUSION;
		break;
	case Type::PIPELINE_STATISTICS:
		type = D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
		break;
	case Type::TIMESTAMP:
		type = D3D12_QUERY_TYPE_TIMESTAMP;
		break;
	default:
		break;
	}

	return type;
}

double QueryPool::ConvertTimingDataMilliseconds(uint64_t datum)
{
	MIRU_CPU_PROFILE_FUNCTION();

	const float& timestampPeriod = 1.0f;//ref_cast<PhysicalDevice>(m_CI.device->GetCreateInfo().physicalDevice)->m_Properties.limits.timestampPeriod;
	return (static_cast<double>(datum) / static_cast<double>(timestampPeriod)) / 1000000.0;
}
