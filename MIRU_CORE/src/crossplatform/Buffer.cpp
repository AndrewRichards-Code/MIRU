#include "common.h"
#include "directx12/D3D12Buffer.h"
#include "vulkan/VKBuffer.h"

using namespace miru;
using namespace crossplatform;

Ref<Buffer> Buffer::Create(Buffer::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return std::make_shared<d3d12::Buffer>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return std::make_shared<vulkan::Buffer>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<BufferView> BufferView::Create(BufferView::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return std::make_shared<d3d12::BufferView>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return std::make_shared<vulkan::BufferView>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}