#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Sync.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKSync.h"
#endif

using namespace miru;
using namespace base;

FenceRef Fence::Create(Fence::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Fence>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Fence>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

SemaphoreRef Semaphore::Create(Semaphore::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Semaphore>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Semaphore>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

EventRef Event::Create(Event::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Event>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Event>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

BarrierRef Barrier::Create(Barrier::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Barrier>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Barrier>(pCreateInfo);
		#else
		return nullptr;
		#endif
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

Barrier2Ref Barrier2::Create(Barrier2::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Barrier2>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Barrier2>(pCreateInfo);
		#else
		return nullptr;
		#endif
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}