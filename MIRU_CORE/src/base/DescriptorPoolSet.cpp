#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12DescriptorPoolSet.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKDescriptorPoolSet.h"
#endif

using namespace miru;
using namespace base;

DescriptorPoolRef DescriptorPool::Create(DescriptorPool::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::DescriptorPool>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::DescriptorPool>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

DescriptorSetLayoutRef DescriptorSetLayout::Create(DescriptorSetLayout::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::DescriptorSetLayout>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::DescriptorSetLayout>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

DescriptorSetRef DescriptorSet::Create(DescriptorSet::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::DescriptorSet>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::DescriptorSet>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}