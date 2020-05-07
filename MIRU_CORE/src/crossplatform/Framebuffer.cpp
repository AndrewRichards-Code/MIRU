#include "miru_core_common.h"
#include "directx12/D3D12Framebuffer.h"
#include "vulkan/VKFramebuffer.h"

using namespace miru;
using namespace crossplatform;

Ref<Framebuffer> Framebuffer::Create(Framebuffer::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Framebuffer>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Framebuffer>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}