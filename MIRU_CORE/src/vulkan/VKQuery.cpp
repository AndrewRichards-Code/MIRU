#include "VKQuery.h"
#include "VKDevice.h"
#include "VKPhysicalDevice.h"

using namespace miru;
using namespace vulkan;

QueryPool::QueryPool(CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Device>(pCreateInfo->device)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	
	m_QueryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	m_QueryPoolCI.pNext = nullptr;
	m_QueryPoolCI.flags = m_CI.reset ? VK_QUERY_POOL_CREATE_RESET_BIT_KHR : VkQueryPoolCreateFlags(0);
	m_QueryPoolCI.queryType = static_cast<VkQueryType>(m_CI.type);
	m_QueryPoolCI.queryCount = m_CI.count;
	m_QueryPoolCI.pipelineStatistics = static_cast<VkQueryPipelineStatisticFlags>(m_CI.pipelineStatisticFlags);

	MIRU_FATAL(vkCreateQueryPool(m_Device, &m_QueryPoolCI, nullptr, &m_QueryPool), "ERROR: VULKAN: Failed to create QueryPool.");
	VKSetName<VkQueryPool>(m_Device, m_QueryPool, m_CI.debugName);

	CreateReadbackBuffer();
}

QueryPool::~QueryPool()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyQueryPool(m_Device, m_QueryPool, nullptr);
}

void QueryPool::Reset(uint32_t firstQuery, uint32_t queryCount)
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkResetQueryPool(m_Device, m_QueryPool, firstQuery, queryCount);
}

double QueryPool::ConvertTimingDataMilliseconds(uint64_t datum)
{
	MIRU_CPU_PROFILE_FUNCTION();

	const float& timestampPeriod = ref_cast<PhysicalDevice>(m_CI.device->GetCreateInfo().physicalDevice)->m_Properties.limits.timestampPeriod;
	return (static_cast<double>(datum) * static_cast<double>(timestampPeriod)) / 1000000.0;
}