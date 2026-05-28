#pragma once
#include "base/Query.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class QueryPool final : public base::QueryPool
	{
		//Methods
	public:
		QueryPool(QueryPool::CreateInfo* pCreateInfo);
		~QueryPool();

		void Reset(uint32_t firstQuery, uint32_t queryCount) override;
		double ConvertTimingDataMilliseconds(uint64_t datum) override;

		D3D12_QUERY_TYPE GetQueryType();

		//Members
	public:
		ID3D12Device* m_Device;

		ID3D12QueryHeap* m_QueryHeap;
		D3D12_QUERY_HEAP_DESC m_QueryHeapDesc;
	};
}
}