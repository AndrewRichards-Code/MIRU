#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Query.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKQuery.h"
#endif

#include "Buffer.h"

using namespace miru;
using namespace base;

QueryPoolRef QueryPool::Create(QueryPool::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
#if defined (MIRU_D3D12)
		return CreateRef<d3d12::QueryPool>(pCreateInfo);
#else
		return nullptr;
#endif
	case GraphicsAPI::API::VULKAN:
#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::QueryPool>(pCreateInfo);
#else
		return nullptr;
#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

void QueryPool::CreateReadbackBuffer()
{
	Buffer::CreateInfo readbackBufferCI;
	readbackBufferCI.debugName = m_CI.debugName + " - Readback Buffer";
	readbackBufferCI.device = m_CI.device;
	readbackBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT;
	readbackBufferCI.imageDimension = {};
	readbackBufferCI.size = m_CI.count * sizeof(uint64_t);
	readbackBufferCI.data = nullptr;
	readbackBufferCI.allocator = m_CI.allocatorCPU;
	m_ReadbackBuffer = Buffer::Create(&readbackBufferCI);
}
