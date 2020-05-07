#include "miru_core_common.h"
#include "directx12/D3D12Sync.h"
#include "vulkan/VKSync.h"

using namespace miru;
using namespace crossplatform;

Ref<Fence> Fence::Create(Fence::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Fence>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Fence>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<Semaphore> Semaphore::Create(Semaphore::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Semaphore>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Semaphore>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<Event> Event::Create(Event::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Event>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Event>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<Barrier> Barrier::Create(Barrier::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Barrier>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Barrier>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}