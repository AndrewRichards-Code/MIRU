#pragma once
#include "base/Query.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class QueryPool final : public base::QueryPool
	{
		//Methods
	public:
		QueryPool(QueryPool::CreateInfo* pCreateInfo);
		~QueryPool();

		void Reset(uint32_t firstQuery, uint32_t queryCount) override;
		double ConvertTimingDataMilliseconds(uint64_t datum) override;

		//Members
	public:
		VkDevice m_Device;

		VkQueryPool m_QueryPool;
		VkQueryPoolCreateInfo m_QueryPoolCI;
	};
}
}