#include "miru_core_common.h"
#include "directx12/D3D12CommandPoolBuffer.h"
#include "vulkan/VKCommandPoolBuffer.h"

using namespace miru;
using namespace crossplatform;

Ref<CommandPool> CommandPool::Create(CommandPool::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::CommandPool>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::CommandPool>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<CommandBuffer> CommandBuffer::Create(CommandBuffer::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::CommandBuffer>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::CommandBuffer>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}