#pragma once
#include "crossplatform/DescriptorPoolSet.h"

namespace miru
{
namespace d3d12
{
	class DescriptorPool final : public crossplatform::DescriptorPool
	{
		//Methods
	public:
		DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo);
		~DescriptorPool();

		//Members
	public:
		ID3D12Device* m_Device;

		//Per Set per type
		//[i][0] == HEAP_TYPE_CBV_SRV_UAV	and [i][1] == HEAP_TYPE_SAMPLER
		//[i][2] == HEAP_TYPE_RTV			and [i][3] == HEAP_TYPE_DSV
		std::vector<std::array<ID3D12DescriptorHeap*, 4>> m_DescriptorPools; 
		D3D12_DESCRIPTOR_HEAP_DESC m_DescriptorPoolDescs[4];
	};

	class DescriptorSetLayout final : public crossplatform::DescriptorSetLayout
	{
		//Methods
	public:
		DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo);
		~DescriptorSetLayout();

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_ROOT_DESCRIPTOR_TABLE m_DescriptorTable = { 0, nullptr };
		D3D12_ROOT_DESCRIPTOR_TABLE m_DescriptorTableSampler = { 0, nullptr };
	};

	class DescriptorSet final : public crossplatform::DescriptorSet //DescriptorTables???
	{
		//Methods
	public:
		DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo);
		~DescriptorSet();

		void AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		void AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is index offset into that array.
		void Update() override;

		//Members
	public:
		ID3D12Device* m_Device;

		//Per Set per type
		//[i][0] == HEAP_TYPE_CBV_SRV_UAV	and [i][1] == HEAP_TYPE_SAMPLER
		//[i][2] == HEAP_TYPE_RTV			and [i][3] == HEAP_TYPE_DSV
		std::vector<std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4>> m_DescHeapBaseCPUHandles;
		//Per Set per binding per type
		//[set][binding][0] == HEAP_TYPE_CBV_SRV_UAV	and [set][binding][1] == HEAP_TYPE_SAMPLER
		//[set][binding][2] == HEAP_TYPE_RTV			and [set][binding][3] == HEAP_TYPE_DSV
		std::map<uint32_t, std::map<uint32_t, std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4>>> m_DescCPUHandles;

		//Per Set per available type in the potential order:
		//RANGE_TYPE_SRV, RANGE_TYPE_UAV and RANGE_TYPE_CBV
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_DescriptorRanges;
		std::vector<D3D12_ROOT_DESCRIPTOR_TABLE> m_DescriptorTables;
		
		//Per Set per type RANGE_TYPE_SAMPLER:
		std::vector<D3D12_DESCRIPTOR_RANGE> m_DescriptorRangeSampler;
		D3D12_ROOT_DESCRIPTOR_TABLE m_DescriptorTableSampler;
		
		std::vector<D3D12_ROOT_PARAMETER> m_RootParameters;
	};

}
}