#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Framebuffer.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKFramebuffer.h"
#endif

using namespace miru;
using namespace base;

FramebufferRef Framebuffer::Create(Framebuffer::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Framebuffer>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Framebuffer>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}