#pragma once
#include "base/DescriptorPoolSet.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class DescriptorPool final : public base::DescriptorPool
	{
		//Methods
	public:
		DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo);
		~DescriptorPool();

		//Members
	public:
		ID3D12Device* m_Device;

		size_t m_AssignedSets = 0;
	};

	class DescriptorSetLayout final : public base::DescriptorSetLayout
	{
		//Methods
	public:
		DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo);
		~DescriptorSetLayout();

		//Members
	public:
		ID3D12Device* m_Device;

		//Valid if NumDescriptors > 0.
		//BaseShaderRegister and RegisterSpace with values ~0U are unknowns.
		std::array<D3D12_DESCRIPTOR_RANGE, 4> m_DescriptorRanges;
	};

	class DescriptorSet final : public base::DescriptorSet
	{
		//Methods
	public:
		DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo);
		~DescriptorSet();

		void AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		void AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		void AddAccelerationStructure(uint32_t index, uint32_t bindingIndex, const std::vector<base::AccelerationStructureRef>& accelerationStructures, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		void Update() override;

		//Members
	public:
		ID3D12Device* m_Device;

		//Per Index per type
		//[index][0] == HEAP_TYPE_CBV_SRV_UAV and [index][1] == HEAP_TYPE_SAMPLER
		//[index][2] == HEAP_TYPE_RTV         and [index][3] == HEAP_TYPE_DSV
		std::vector<std::array<ID3D12DescriptorHeap*, 4>> m_DescriptorHeaps;
		std::vector<std::array<D3D12_DESCRIPTOR_HEAP_DESC, 4>> m_DescriptorHeapDescs;

		//Per Index per binding per type
		//[index][binding][0] == HEAP_TYPE_CBV_SRV_UAV and [index][binding][1] == HEAP_TYPE_SAMPLER
		//[index][binding][2] == HEAP_TYPE_RTV         and [index][binding][3] == HEAP_TYPE_DSV
		std::map<uint32_t, std::map<uint32_t, std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4>>> m_DescCPUHandles;
	};

}
}