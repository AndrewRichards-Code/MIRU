#include "common.h"
#include "directx12/D3D12Context.h"
#include "vulkan/VKContext.h"

using namespace miru;
using namespace crossplatform;

Ref<Context> Context::Create(Context::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return std::make_shared<d3d12::Context>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return std::make_shared<vulkan::Context>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}