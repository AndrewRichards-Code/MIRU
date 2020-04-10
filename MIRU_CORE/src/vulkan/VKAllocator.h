#pragma once
#include "miru_core_common.h"
#include "crossplatform/Allocator.h"

namespace miru
{
namespace vulkan
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
		VkMemoryPropertyFlags GetMemoryPropertyFlag(crossplatform::Resource::Type type, uint32_t usage);
		uint32_t GetMemoryTypeIndex(VkMemoryPropertyFlags properties);
		uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueType);

		//Members
	private:
		VkDevice m_Device;

		VkDeviceMemory m_DeviceMemory;
		VkMemoryAllocateInfo m_AI;

		uint32_t m_MemoryTypeIndex = 0;

		static VkPhysicalDeviceProperties s_PhysicalDeviceProperties;
		static VkPhysicalDeviceMemoryProperties s_PhysicalDeviceMemoryProperties;
		static uint32_t s_MaxAllocations;
		static uint32_t s_CurrentAllocations;
	};
}
}