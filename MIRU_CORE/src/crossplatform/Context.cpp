#include "miru_core_common.h"
#include "directx12/D3D12Context.h"
#include "vulkan/VKContext.h"

using namespace miru;
using namespace crossplatform;

Ref<Context> Context::Create(Context::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::Context>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::Context>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}