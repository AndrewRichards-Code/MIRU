#include "common.h"
#include "directx12/D3D12Swapchain.h"
#include "vulkan/VKSwapchain.h"

using namespace miru;
using namespace crossplatform;

Ref<Swapchain> Swapchain::Create(Swapchain::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return std::make_shared<d3d12::Swapchain>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return std::make_shared<vulkan::Swapchain>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}