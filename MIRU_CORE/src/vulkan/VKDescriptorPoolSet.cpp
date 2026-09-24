#include "VKDescriptorPoolSet.h"
#include "VKDevice.h"
#include "VKBuffer.h"
#include "VKImage.h"
#include "VKAccelerationStructure.h"

using namespace miru;
using namespace vulkan;

//DescriptorPool
DescriptorPool::DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Device>(pCreateInfo->device)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	bool descriptorIndexing = arc::BitwiseCheck(m_CI.device->GetResultInfo().activeExtensions, Device::ExtensionsBit::DESCRIPTOR_INDEXING);

	for (auto& poolSize : m_CI.poolSizes)
		m_PoolSizes.push_back({ static_cast<VkDescriptorType>(poolSize.type), poolSize.descriptorCount });

	m_DescriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	m_DescriptorPoolCI.pNext = nullptr;
	m_DescriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | (m_CI.updateAfterBind && descriptorIndexing ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT : VkDescriptorPoolCreateFlags(0));
	m_DescriptorPoolCI.maxSets = m_CI.maxSets;
	m_DescriptorPoolCI.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
	m_DescriptorPoolCI.pPoolSizes = m_PoolSizes.data();

	MIRU_FATAL(vkCreateDescriptorPool(m_Device, &m_DescriptorPoolCI, nullptr, &m_DescriptorPool), "ERROR: VULKAN: Failed to create DescriptorPool.");
	VKSetName<VkDescriptorPool>(m_Device, m_DescriptorPool, m_CI.debugName);
}

DescriptorPool::~DescriptorPool()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
}

//DescriptorSetLayout
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Device>(pCreateInfo->device)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	bool descriptorIndexing = arc::BitwiseCheck(m_CI.device->GetResultInfo().activeExtensions, Device::ExtensionsBit::DESCRIPTOR_INDEXING);

	for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayoutBinding)
	{
		m_DescriptorSetLayoutBindings.push_back({
		descriptorSetLayoutBinding.binding,
		static_cast<VkDescriptorType>(descriptorSetLayoutBinding.type),
		descriptorSetLayoutBinding.descriptorCount,
		static_cast<VkShaderStageFlags>(descriptorSetLayoutBinding.stage),
		nullptr });

		if (descriptorIndexing)
		{
			m_DescriptorSetLayoutBindingFlags.push_back({
			static_cast<VkDescriptorBindingFlags>(descriptorSetLayoutBinding.flags)
			});

			//DescriptorUnboundedArrayCount maybe exceed the device limits
			if (m_DescriptorSetLayoutBindings.back().descriptorCount == base::DescriptorUnboundedArrayCount
				&& arc::BitwiseCheck(m_DescriptorSetLayoutBindingFlags.back(), static_cast<VkDescriptorBindingFlags>(Binding::FlagBit::VARIABLE_DESCRIPTOR_COUNT_BIT)))
			{
				uint32_t descriptorCount = 0;
				const Device::FeaturesAndProperties& featureAndProperties = ref_cast<Device>(m_CI.device)->m_FeatureAndProperties;
				switch (m_DescriptorSetLayoutBindings.back().descriptorType)
				{
				default:
					case VK_DESCRIPTOR_TYPE_SAMPLER:
					case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						descriptorCount = featureAndProperties.m_Properties2.properties.limits.maxPerStageDescriptorSamplers;
						break;
					case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						descriptorCount = featureAndProperties.m_Properties2.properties.limits.maxPerStageDescriptorSampledImages;
						break;
					case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						descriptorCount = featureAndProperties.m_Properties2.properties.limits.maxPerStageDescriptorStorageImages;
						break;
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
						descriptorCount = featureAndProperties.m_Properties2.properties.limits.maxPerStageDescriptorUniformBuffers;
						break;
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
						descriptorCount = featureAndProperties.m_Properties2.properties.limits.maxPerStageDescriptorStorageBuffers;
						break;
					case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
						descriptorCount = featureAndProperties.m_Properties2.properties.limits.maxPerStageDescriptorInputAttachments;
						break;
					case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
						descriptorCount = featureAndProperties.m_Vulkan13Properties.maxPerStageDescriptorInlineUniformBlocks;
						break;
					case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
						descriptorCount = featureAndProperties.m_AccelerationStructureProperties.maxPerStageDescriptorAccelerationStructures;
						break;
				};
				m_DescriptorSetLayoutBindings.back().descriptorCount = descriptorCount;
			}
		}
	}

	m_DescriptorSetLayoutBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	m_DescriptorSetLayoutBindingFlagsCreateInfo.pNext = nullptr;
	m_DescriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(m_DescriptorSetLayoutBindingFlags.size());
	m_DescriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = m_DescriptorSetLayoutBindingFlags.data();

	m_DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	m_DescriptorSetLayoutCI.pNext = descriptorIndexing ? &m_DescriptorSetLayoutBindingFlagsCreateInfo : nullptr;
	m_DescriptorSetLayoutCI.flags = (m_CI.updateAfterBind && descriptorIndexing ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT : VkDescriptorSetLayoutCreateFlags(0));
	m_DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(m_DescriptorSetLayoutBindings.size());
	m_DescriptorSetLayoutCI.pBindings = m_DescriptorSetLayoutBindings.data();

	MIRU_FATAL(vkCreateDescriptorSetLayout(m_Device, &m_DescriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout), "ERROR: VULKAN: Failed to create DescriptorSetLayout.");
	VKSetName<VkDescriptorSetLayout>(m_Device, m_DescriptorSetLayout, m_CI.debugName);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

//DescriptorSet
DescriptorSet::DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo)
	:m_Device(ref_cast<Device>(pCreateInfo->descriptorPool->GetCreateInfo().device)->m_Device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	bool descriptorIndexing = arc::BitwiseCheck(pCreateInfo->descriptorPool->GetCreateInfo().device->GetResultInfo().activeExtensions, Device::ExtensionsBit::DESCRIPTOR_INDEXING);

	if (descriptorIndexing)
	{
		m_DescriptorSetVariableDescriptorCountAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		m_DescriptorSetVariableDescriptorCountAI.pNext = nullptr;
		m_DescriptorSetVariableDescriptorCountAI.descriptorSetCount = static_cast<uint32_t>(m_CI.descriptorCounts.size());
		m_DescriptorSetVariableDescriptorCountAI.pDescriptorCounts = m_CI.descriptorCounts.data();
	}

	for (auto& descriptorSetLayout : m_CI.descriptorSetLayouts)
		m_DescriptorSetLayouts.push_back(ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorSetLayout);

	m_DescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	m_DescriptorSetAI.pNext = descriptorIndexing ? &m_DescriptorSetVariableDescriptorCountAI : nullptr;
	m_DescriptorSetAI.descriptorPool = ref_cast<DescriptorPool>(m_CI.descriptorPool)->m_DescriptorPool;
	m_DescriptorSetAI.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
	m_DescriptorSetAI.pSetLayouts = m_DescriptorSetLayouts.data();

	m_DescriptorSets.resize(m_DescriptorSetLayouts.size());

	MIRU_FATAL(vkAllocateDescriptorSets(m_Device, &m_DescriptorSetAI, m_DescriptorSets.data()), "ERROR: VULKAN: Failed to create DescriptorSet.");
	
	uint32_t i = 0;
	for (auto& descriptorSet : m_DescriptorSets)
	{
		VKSetName<VkDescriptorSet>(m_Device, descriptorSet, m_CI.debugName + ": " + std::to_string(i));
		i++;
	}
}

DescriptorSet::~DescriptorSet()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkFreeDescriptorSets(m_Device, m_DescriptorSetAI.descriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());
}

void DescriptorSet::AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t descriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	for (auto& descriptorBufferInfo : descriptorBufferInfos)
	{
		m_DescriptorBufferInfo[index][bindingIndex].push_back({
			ref_cast<Buffer>(ref_cast<BufferView>(descriptorBufferInfo.bufferView)->GetCreateInfo().buffer)->m_Buffer,
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_BufferViewCI.offset,
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_BufferViewCI.range
			});
	}

	base::DescriptorType descriptorType = base::DescriptorType(0);
	for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayouts[index]->GetCreateInfo().descriptorSetLayoutBinding)
	{
		if (descriptorSetLayoutBinding.binding == bindingIndex)
		{
			descriptorType = descriptorSetLayoutBinding.type;
			break;
		}
	}

	VkWriteDescriptorSet wds;
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = nullptr;
	wds.dstSet = m_DescriptorSets[index];
	wds.dstBinding = bindingIndex;
	wds.dstArrayElement = descriptorArrayIndex;
	wds.descriptorCount = static_cast<uint32_t>(m_DescriptorBufferInfo[index][bindingIndex].size());
	wds.descriptorType = static_cast<VkDescriptorType>(descriptorType);
	wds.pImageInfo = nullptr;
	wds.pBufferInfo = m_DescriptorBufferInfo[index][bindingIndex].data();
	wds.pTexelBufferView = nullptr;

	m_WriteDescriptorSets.push_back(wds);
}

void DescriptorSet::AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t descriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	for (auto& descriptorImageInfo : descriptorImageInfos)
	{
		m_DescriptorImageInfo[index][bindingIndex].push_back({
			descriptorImageInfo.sampler ? ref_cast<Sampler>(descriptorImageInfo.sampler)->m_Sampler : VK_NULL_HANDLE,
			descriptorImageInfo.imageView ? ref_cast<ImageView>(descriptorImageInfo.imageView)->m_ImageView : VK_NULL_HANDLE,
			static_cast<VkImageLayout>(descriptorImageInfo.imageLayout)
			});
	}

	base::DescriptorType descriptorType = base::DescriptorType(0);
	for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayouts[index]->GetCreateInfo().descriptorSetLayoutBinding)
	{
		if (descriptorSetLayoutBinding.binding == bindingIndex)
		{
			descriptorType = descriptorSetLayoutBinding.type;
			break;
		}
	}

	VkWriteDescriptorSet wds;
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = nullptr;
	wds.dstSet = m_DescriptorSets[index];
	wds.dstBinding = bindingIndex;
	wds.dstArrayElement = descriptorArrayIndex;
	wds.descriptorCount = static_cast<uint32_t>(m_DescriptorImageInfo[index][bindingIndex].size());
	wds.descriptorType = static_cast<VkDescriptorType>(descriptorType);
	wds.pImageInfo = m_DescriptorImageInfo[index][bindingIndex].data();
	wds.pBufferInfo = nullptr;
	wds.pTexelBufferView = nullptr;

	m_WriteDescriptorSets.push_back(wds);
}

void DescriptorSet::AddAccelerationStructure(uint32_t index, uint32_t bindingIndex, const std::vector<base::AccelerationStructureRef>& accelerationStructures, uint32_t descriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	for (auto& accelerationStructure : accelerationStructures)
	{
		m_AccelerationStructures[index][bindingIndex].push_back(ref_cast<AccelerationStructure>(accelerationStructure)->m_AS);
	}

	VkWriteDescriptorSetAccelerationStructureKHR& wdsas = m_WriteDescriptorSetAccelerationStructure[index][bindingIndex];
	wdsas.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	wdsas.pNext = nullptr;
	wdsas.accelerationStructureCount = static_cast<uint32_t>(m_AccelerationStructures.size());
	wdsas.pAccelerationStructures = m_AccelerationStructures[index][bindingIndex].data();

	VkWriteDescriptorSet wds;
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = &(m_WriteDescriptorSetAccelerationStructure[index][bindingIndex]);
	wds.dstSet = m_DescriptorSets[index];
	wds.dstBinding = bindingIndex;
	wds.dstArrayElement = descriptorArrayIndex;
	wds.descriptorCount = m_WriteDescriptorSetAccelerationStructure[index][bindingIndex].accelerationStructureCount;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	wds.pImageInfo = nullptr;
	wds.pBufferInfo = nullptr;
	wds.pTexelBufferView = nullptr;

	m_WriteDescriptorSets.push_back(wds);
}

void DescriptorSet::Update()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(m_WriteDescriptorSets.size()), m_WriteDescriptorSets.data(), 0, nullptr);
}

void DescriptorSet::Clear()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_WriteDescriptorSets.clear();

	auto ClearMapMapVector = []<typename T>(std::map<uint32_t, std::map<uint32_t, std::vector<T>>>& container) -> void
		{
			for (auto& a : container)
			{
				for (auto& b : a.second)
				{
					b.second.clear();
				}
				a.second.clear();
			}
			container.clear();
		};
	auto ClearMapMap = []<typename T>(std::map<uint32_t, std::map<uint32_t, T>>&container) -> void
	{
		for (auto& a : container)
		{
			a.second.clear();
		}
		container.clear();
	};

	ClearMapMapVector.template operator()<VkDescriptorBufferInfo>(m_DescriptorBufferInfo);
	ClearMapMapVector.template operator()<VkDescriptorImageInfo>(m_DescriptorImageInfo);

	ClearMapMapVector.template operator()<VkAccelerationStructureKHR>(m_AccelerationStructures);
	ClearMapMap.template operator()<VkWriteDescriptorSetAccelerationStructureKHR>(m_WriteDescriptorSetAccelerationStructure);

}