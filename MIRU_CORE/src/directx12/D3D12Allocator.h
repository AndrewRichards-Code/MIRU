#pragma once
#include "common.h"
#include "crossplatform/Allocator.h"

namespace miru
{
namespace d3d12
{
	class MemoryBlock : public crossplatform::MemoryBlock
	{
		//Methods
	public:
		MemoryBlock(MemoryBlock::CreateInfo* pCreateInfo);
		~MemoryBlock();
		bool AddResource(crossplatform::Resource& resource) override;
		void RemoveResource(uint64_t id) override;
		void SubmitData(const crossplatform::Resource& resource, size_t size, void* data) override;

	private:
		D3D12_HEAP_PROPERTIES GetHeapProperties(crossplatform::MemoryBlock::PropertiesBit properties);
		uint32_t GetMemoryTypeIndex(VkMemoryPropertyFlags properties);
		uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueType);

		//Members
	private:
		ID3D12Device* m_Device;


	public:
		ID3D12Heap* m_MemoryHeap;
		D3D12_HEAP_DESC m_HeapDesc;
	};
}
}