#pragma once
#include "base/Allocator.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class Allocator final : public base::Allocator
	{
		//Methods
	public:
		Allocator(Allocator::CreateInfo* pCreateInfo);
		~Allocator();

		void* GetNativeAllocator() override;

		void SubmitData(const base::Allocation& allocation, size_t offset, size_t size, void* data) override;
		void AccessData(const base::Allocation& allocation, size_t offset, size_t size, void* data) override;

	private:
		/*VkMemoryPropertyFlags GetMemoryPropertyFlag(base::Resource::Type type, uint32_t usage);
		uint32_t GetMemoryTypeIndex(VkMemoryPropertyFlags properties);
		uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueType);*/

		//Members
	private:
		VkDevice m_Device;

		VmaAllocator m_Allocator;
		VmaAllocatorCreateInfo m_AI;
	};
}
}