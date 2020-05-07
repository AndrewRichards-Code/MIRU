#include "miru_core_common.h"
#include "directx12/D3D12Allocator.h"
#include "vulkan/VKAllocator.h"

using namespace miru;
using namespace crossplatform;

std::vector<MemoryBlock*> MemoryBlock::s_MemoryBlocks = {};
std::map<MemoryBlock*, std::map<uint64_t, Resource>> MemoryBlock::s_AllocatedResources = {};
uint64_t MemoryBlock::ruid_src = 0;

Ref<MemoryBlock> MemoryBlock::Create(MemoryBlock::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return CreateRef<d3d12::MemoryBlock>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return CreateRef<vulkan::MemoryBlock>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

void MemoryBlock::CalculateOffsets()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto it = s_AllocatedResources[this].begin(); it != s_AllocatedResources[this].end(); it++)
	{
		auto& resource = *it;
		auto& prev = it != s_AllocatedResources[this].begin() ? *std::prev(it) : *it;

		if (it == s_AllocatedResources[this].begin())
			resource.second.offset = 0;
		else
		{
			resource.second.offset = prev.second.size + prev.second.offset;
			if (resource.second.offset % resource.second.alignment != 0)
			{
				resource.second.offset = (((resource.second.offset / resource.second.alignment) + 1) * resource.second.alignment);
			}
		}
	}
}

bool MemoryBlock::ResourceBackable(crossplatform::Resource& resource)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CalculateOffsets();
	size_t maxSize = static_cast<size_t>(m_CI.blockSize);
	size_t currentSize = 0;
	if(!s_AllocatedResources[this].empty())
		currentSize = s_AllocatedResources[this].rbegin()->second.offset + s_AllocatedResources[this].rbegin()->second.size;
	return (maxSize > (((currentSize / resource.alignment) + 1) * resource.alignment) + resource.size);
}