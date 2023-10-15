#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Pipeline.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKPipeline.h"
#endif

using namespace miru;
using namespace base;

RenderPassRef RenderPass::Create(RenderPass::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
		case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::RenderPass>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::RenderPass>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

PipelineRef Pipeline::Create(Pipeline::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
		case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Pipeline>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Pipeline>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}