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

		ID3D12DescriptorHeap* m_DescriptorPool[2]; //[0] == HEAP_TYPE_SAMPLER and [1] == HEAP_TYPE_CBV_SRV_UAV
		D3D12_DESCRIPTOR_HEAP_DESC m_DescriptorPoolDesc;
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

		std::vector<D3D12_DESCRIPTOR_RANGE> m_DescriptorRanges;
		D3D12_ROOT_DESCRIPTOR_TABLE m_DescriptorTable
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

	private:
		VkDescriptorType BufferViewTypeToVkDescriptorType(crossplatform::BufferView::Type type);
		VkDescriptorType ImageUsageToVkDescriptorType(crossplatform::Image::UsageBit usage, bool sampler, bool image);

		//Members
	public:
		VkDevice& m_Device;

		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkDescriptorSetAllocateInfo m_DescriptorSetAI;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;
		std::map<uint32_t, std::map<uint32_t, std::vector<VkDescriptorBufferInfo>>> m_DescriptorBufferInfo;
		std::map<uint32_t, std::map<uint32_t, std::vector<VkDescriptorImageInfo>>> m_DescriptorImageInfo;
	};
}
}