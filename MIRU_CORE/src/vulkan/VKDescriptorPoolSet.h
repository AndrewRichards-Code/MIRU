#pragma once
#include "base/DescriptorPoolSet.h"
#include "base/Buffer.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class DescriptorPool final : public base::DescriptorPool
	{
		//Methods
	public:
		DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo);
		~DescriptorPool();

		//Members
	public:
		VkDevice& m_Device;

		VkDescriptorPool m_DescriptorPool;
		VkDescriptorPoolCreateInfo m_DescriptorPoolCI;
		std::vector<VkDescriptorPoolSize> m_PoolSizes;
	};

	class DescriptorSetLayout final : public base::DescriptorSetLayout
	{
		//Methods
	public:
		DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo);
		~DescriptorSetLayout();

		//Members
	public:
		VkDevice& m_Device;

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorSetLayoutCreateInfo m_DescriptorSetLayoutCI;
		std::vector<VkDescriptorSetLayoutBinding> m_DescriptorSetLayoutBindings;
	};

	class DescriptorSet final : public base::DescriptorSet
	{
		//Methods
	public:
		DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo);
		~DescriptorSet();

		void AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is the base index in that array.
		void AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is the base index in that array.
		void AddAccelerationStructure(uint32_t index, uint32_t bindingIndex, const std::vector<base::AccelerationStructureRef>& accelerationStructures, uint32_t desriptorArrayIndex = 0) override; //If descriptor is an array, desriptorArrayIndex is the base index in that array.
		void Update() override;

		//Members
	public:
		VkDevice& m_Device;

		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkDescriptorSetAllocateInfo m_DescriptorSetAI;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

		std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;
		std::map<uint32_t, std::map<uint32_t, std::vector<VkDescriptorBufferInfo>>> m_DescriptorBufferInfo;
		std::map<uint32_t, std::map<uint32_t, std::vector<VkDescriptorImageInfo>>> m_DescriptorImageInfo;

		std::map<uint32_t, std::map<uint32_t, std::vector<VkAccelerationStructureKHR>>> m_AccelerationStructures;
		std::map<uint32_t, std::map<uint32_t, VkWriteDescriptorSetAccelerationStructureKHR>> m_WriteDescriptorSetAccelerationStructure;
	};
}
}