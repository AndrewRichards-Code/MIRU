#pragma once
#if defined(MIRU_D3D12)
#include "base/Sync.h"

namespace miru
{
namespace d3d12
{
	class Fence final : public base::Fence
	{
		//Methods
	public:
		Fence(base::Fence::CreateInfo* pCreateInfo);
		~Fence();

		void Reset() override;
		bool GetStatus() override;
		bool Wait() override;

		void Signal();

		UINT64& GetValue() { return m_Value; }

		//Members
	public:
		ID3D12Device* m_Device;
		ID3D12Fence* m_Fence;

	private:
		UINT64 m_Value;
		HANDLE m_Event;
	};

	class Semaphore final : public base::Semaphore
	{
		//Methods
	public:
		Semaphore(base::Semaphore::CreateInfo* pCreateInfo);
		~Semaphore();

		void Signal(uint64_t value) override;
		bool Wait(uint64_t value, uint64_t timeout) override;
		uint64_t GetCurrentValue() override;
		
		UINT64& GetValue() { return m_Value; }

		//Members
	public:
		ID3D12Device* m_Device;
		ID3D12Fence* m_Semaphore;
	
	private:
		UINT64 m_Value;
	};

	class Event final : public base::Event
	{
		//Methods
	public:
		Event(base::Event::CreateInfo* pCreateInfo);
		~Event();

		void Set() override;
		void Reset() override;
		bool GetStatus() override;

		//Members
	public:
	};

	class Barrier final : public base::Barrier
	{
		//Methods
	public:
		Barrier(base::Barrier::CreateInfo* pCreateInfo);
		~Barrier();

		static D3D12_RESOURCE_STATES ToD3D12ResourceState(Barrier::AccessBit access);

		//Members
	public:
		//Index via arrayIndex * mipCount + mipIndex https://docs.microsoft.com/en-us/windows/win32/direct3d12/subresources#multiple-subresources
		std::vector<D3D12_RESOURCE_BARRIER> m_Barriers;
	};

	class Barrier2 final : public base::Barrier2
	{
		//Methods
	public:
		Barrier2(base::Barrier2::CreateInfo* pCreateInfo);
		~Barrier2();

		//Members
	public:
		//Index via arrayIndex * mipCount + mipIndex https://docs.microsoft.com/en-us/windows/win32/direct3d12/subresources#multiple-subresources
		std::vector<D3D12_RESOURCE_BARRIER> m_Barriers;
	};
}
}
#endif