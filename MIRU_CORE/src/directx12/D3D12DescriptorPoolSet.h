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
		//[set][0] == HEAP_TYPE_CBV_SRV_UAV and [set][1] == HEAP_TYPE_SAMPLER
		//[set][2] == HEAP_TYPE_RTV         and [set][3] == HEAP_TYPE_DSV
		std::vector<std::array<ID3D12DescriptorHeap*, 4>> m_DescriptorPools; 
		D3D12_DESCRIPTOR_HEAP_DESC m_DescriptorPoolDescs[4];

		//Per Set per binding per type
		//[set][binding][0] == HEAP_TYPE_CBV_SRV_UAV and [set][binding][1] == HEAP_TYPE_SAMPLER
		//[set][binding][2] == HEAP_TYPE_RTV         and [set][binding][3] == HEAP_TYPE_DSV
		std::map<uint32_t, std::map<uint32_t, std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4>>> m_DescCPUHandles;
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

		//Valid if NumDescriptors > 0.
		//BaseShaderRegister and RegisterSpace with values ~0U are unknowns.
		std::array<D3D12_DESCRIPTOR_RANGE, 4> m_DescriptorRanges;
	};

	class DescriptorSet final : public crossplatform::DescriptorSet
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

		std::map<uint32_t, std::map<uint32_t, uint32_t>> m_SamplerBindings;
	};

}
}