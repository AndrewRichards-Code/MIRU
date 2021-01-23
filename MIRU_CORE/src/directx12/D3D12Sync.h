#pragma once
#if defined(MIRU_D3D12)
#include "crossplatform/Sync.h"

namespace miru
{
namespace d3d12
{
	class Fence final : public crossplatform::Fence
	{
		//Methods
	public:
		Fence(crossplatform::Fence::CreateInfo* pCreateInfo);
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

	class Semaphore final : public crossplatform::Semaphore
	{
		//Methods
	public:
		Semaphore(crossplatform::Semaphore::CreateInfo* pCreateInfo);
		~Semaphore();

		UINT64& GetValue() { return m_Value; }

		//Members
	public:
		ID3D12Device* m_Device;
		ID3D12Fence* m_Semaphore;
	
	private:
		UINT64 m_Value;
	};

	class Event final : public crossplatform::Event
	{
		//Methods
	public:
		Event(crossplatform::Event::CreateInfo* pCreateInfo);
		~Event();

		void Set() override;
		void Reset() override;
		bool GetStatus() override;

		//Members
	public:
	};

	class Barrier final : public crossplatform::Barrier
	{
		//Methods
	public:
		Barrier(crossplatform::Barrier::CreateInfo* pCreateInfo);
		~Barrier();

	private:
		D3D12_RESOURCE_STATES ToD3D12ResourceState(Barrier::AccessBit access);
		//Members
	public:
		//Index via arrayIndex * mipCount + mipIndex https://docs.microsoft.com/en-us/windows/win32/direct3d12/subresources#multiple-subresources
		std::vector<D3D12_RESOURCE_BARRIER> m_Barriers;
	};
}
}
#endif