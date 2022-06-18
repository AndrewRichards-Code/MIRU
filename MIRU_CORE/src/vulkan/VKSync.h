#pragma once
#if defined(MIRU_VULKAN)
#include "base/Sync.h"

namespace miru
{
namespace vulkan
{
	class Fence final : public base::Fence
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

	class Semaphore final : public base::Semaphore
	{
		//Methods
	public:
		Semaphore(Semaphore::CreateInfo* pCreateInfo);
		~Semaphore();
		
		void Signal(uint64_t value) override;
		bool Wait(uint64_t value, uint64_t timeout) override;
		uint64_t GetCurrentValue() override;

		//Members
	public:
		VkDevice m_Device;

		VkSemaphore m_Semaphore;
		VkSemaphoreCreateInfo m_SemaphoreCI;
		VkSemaphoreTypeCreateInfoKHR m_SemaphoreTypeCI;
	};

	class Event final : public base::Event
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

	class Barrier final : public base::Barrier
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

	class Barrier2 final : public base::Barrier2
	{
		//Methods
	public:
		Barrier2(Barrier2::CreateInfo* pCreateInfo);
		~Barrier2();

		//Members
	public:
		VkMemoryBarrier2 m_MB = {};
		VkBufferMemoryBarrier2 m_BMB = {};
		VkImageMemoryBarrier2 m_IMB = {};
	};
}
}
#endif