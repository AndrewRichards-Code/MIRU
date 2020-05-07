#include "miru_core_common.h"
#include "directx12/D3D12Image.h"
#include "vulkan/VKImage.h"

using namespace miru;
using namespace crossplatform;

Ref<Image> Image::Create(Image::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Image>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Image>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<ImageView> ImageView::Create(ImageView::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::ImageView>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::ImageView>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<Sampler> Sampler::Create(Sampler::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Sampler>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Sampler>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}