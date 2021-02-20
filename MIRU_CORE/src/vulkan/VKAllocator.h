#pragma once
#if defined(MIRU_VULKAN)
#include "miru_core_common.h"
#include "crossplatform/Allocator.h"

namespace miru
{
namespace vulkan
{
	class Allocator final : public crossplatform::Allocator
	{
		//Methods
	public:
		Allocator(Allocator::CreateInfo* pCreateInfo);
		~Allocator();

		void* GetNativeAllocator() override;

		void SubmitData(const crossplatform::Allocation& allocation, size_t size, void* data) override;
		void AccessData(const crossplatform::Allocation& allocation, size_t size, void* data) override;

	private:
		/*VkMemoryPropertyFlags GetMemoryPropertyFlag(crossplatform::Resource::Type type, uint32_t usage);
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
#endif