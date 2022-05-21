#pragma once
#if defined(MIRU_D3D12)
#include "miru_core_common.h"
#include "base/Allocator.h"

namespace miru
{
namespace d3d12
{
	class Allocator final : public base::Allocator
	{
		//Methods
	public:
		Allocator(Allocator::CreateInfo* pCreateInfo);
		~Allocator();

		void* GetNativeAllocator() override;

		void SubmitData(const base::Allocation& allocation, size_t size, void* data) override;
		void AccessData(const base::Allocation& allocation, size_t size, void* data) override;

		D3D12_HEAP_PROPERTIES GetHeapProperties();

		//Members
	private:
		ID3D12Device* m_Device;

	public:
		D3D12MA::Allocator* m_Allocator;
		D3D12MA::ALLOCATOR_DESC m_AllocatorDesc;
	};
}
}
#endif