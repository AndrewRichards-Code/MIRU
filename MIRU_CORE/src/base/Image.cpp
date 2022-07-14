#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Image.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKImage.h"
#endif

using namespace miru;
using namespace base;

ImageRef Image::Create(Image::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Image>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Image>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

ImageViewRef ImageView::Create(ImageView::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::ImageView>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::ImageView>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

SamplerRef Sampler::Create(Sampler::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
		case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Sampler>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Sampler>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}