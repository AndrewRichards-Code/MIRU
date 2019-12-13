#pragma once
#include "crossplatform/Sync.h"

namespace miru
{
namespace vulkan
{
	class Fence final : public crossplatform::Fence
	{
		//Methods
	public:
		Fence(Fence::CreateInfo* pCreateInfo);
		~Fence();

		void Reset() override;
		bool GetStatus() override;
		bool Wait() override;

		//Members
	public:
		VkDevice m_Device;

		VkFence m_Fence;
		VkFenceCreateInfo m_FenceCI;
	};

	class Semaphore final : public crossplatform::Semaphore
	{
		//Methods
	public:
		Semaphore(Semaphore::CreateInfo* pCreateInfo);
		~Semaphore();

		//Members
	public:
		VkDevice m_Device;

		VkSemaphore m_Semaphore;
		VkSemaphoreCreateInfo m_SemaphoreCI;
	};

	class Event final : public crossplatform::Event
	{
		//Methods
	public:
		Event(Event::CreateInfo* pCreateInfo);
		~Event();

		void Set() override;
		void Reset() override;
		bool GetStatus() override;

		//Members
	public:
		VkDevice m_Device;

		VkEvent m_Event;
		VkEventCreateInfo m_EventCI;
	};

	class Barrier final : public crossplatform::Barrier
	{
		//Methods
	public:
		Barrier(Barrier::CreateInfo* pCreateInfo);
		~Barrier();

		//Members
	public:
		VkMemoryBarrier m_MB = {};
		VkBufferMemoryBarrier m_BMB = {};
		VkImageMemoryBarrier m_IMB = {};
	};
}
}