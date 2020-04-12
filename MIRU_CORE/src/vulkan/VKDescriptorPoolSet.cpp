#include "miru_core_common.h"
#include "VKDescriptorPoolSet.h"
#include "VKBuffer.h"
#include "VKImage.h"

using namespace miru;
using namespace vulkan;

//DescriptorPool
DescriptorPool::DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	for (auto& poolSize : m_CI.poolSizes)
		m_PoolSizes.push_back({ static_cast<VkDescriptorType>(poolSize.type), poolSize.descriptorCount });

	m_DescriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	m_DescriptorPoolCI.pNext = nullptr;
	m_DescriptorPoolCI.flags = 0;
	m_DescriptorPoolCI.maxSets = m_CI.maxSets;
	m_DescriptorPoolCI.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
	m_DescriptorPoolCI.pPoolSizes = m_PoolSizes.data();

	MIRU_ASSERT(vkCreateDescriptorPool(m_Device, &m_DescriptorPoolCI, nullptr, &m_DescriptorPool), "ERROR: VULKAN: Failed to create DescriptorPool.");
	VKSetName<VkDescriptorPool>(m_Device, (uint64_t)m_DescriptorPool, m_CI.debugName);
}

DescriptorPool::~DescriptorPool()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
}

//DescriptorSetLayout
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayoutBinding)
		m_DescriptorSetLayoutBindings.push_back({ 
		descriptorSetLayoutBinding.binding,
		static_cast<VkDescriptorType>(descriptorSetLayoutBinding.type),
		descriptorSetLayoutBinding.descriptorCount,
		static_cast<VkShaderStageFlags>(descriptorSetLayoutBinding.stage),
		nullptr});

	m_DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	m_DescriptorSetLayoutCI.pNext = nullptr;
	m_DescriptorSetLayoutCI.flags = 0;
	m_DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(m_DescriptorSetLayoutBindings.size());
	m_DescriptorSetLayoutCI.pBindings = m_DescriptorSetLayoutBindings.data();

	MIRU_ASSERT(vkCreateDescriptorSetLayout(m_Device, &m_DescriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout), "ERROR: VULKAN: Failed to create DescriptorSetLayout.");
	VKSetName<VkDescriptorSetLayout>(m_Device, (uint64_t)m_DescriptorSetLayout, m_CI.debugName);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

//DescriptorSet
DescriptorSet::DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(ref_cast<DescriptorPool>(pCreateInfo->pDescriptorPool)->GetCreateInfo().device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	for (auto& descriptorSetLayout : m_CI.pDescriptorSetLayouts)
		m_DescriptorSetLayouts.push_back(ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorSetLayout);

	m_DescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	m_DescriptorSetAI.pNext = nullptr;
	m_DescriptorSetAI.descriptorPool = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool;
	m_DescriptorSetAI.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
	m_DescriptorSetAI.pSetLayouts = m_DescriptorSetLayouts.data();

	m_DescriptorSets.resize(m_DescriptorSetLayouts.size());

	MIRU_ASSERT(vkAllocateDescriptorSets(m_Device, &m_DescriptorSetAI, m_DescriptorSets.data()), "ERROR: VULKAN: Failed to create DescriptorSet.");
	
	uint32_t i = 0;
	for (auto& descriptorSet : m_DescriptorSets)
	{
		VKSetName<VkDescriptorSetLayout>(m_Device, (uint64_t)descriptorSet, (m_CI.debugName + std::string(" : ") + std::to_string(i)).c_str());
		i++;
	}
}

DescriptorSet::~DescriptorSet()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkFreeDescriptorSets(m_Device, m_DescriptorSetAI.descriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());
}

void DescriptorSet::AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	for (auto& descriptorBufferInfo : descriptorBufferInfos)
	{
		m_DescriptorBufferInfo[index][bindingIndex].push_back({
			ref_cast<Buffer>(ref_cast<BufferView>(descriptorBufferInfo.bufferView)->GetCreateInfo().pBuffer)->m_Buffer,
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_BufferViewCI.offset,
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_BufferViewCI.range
			});
	}

	VkWriteDescriptorSet wds;
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = nullptr;
	wds.dstSet = m_DescriptorSets[index];
	wds.dstBinding = bindingIndex;
	wds.dstArrayElement = desriptorArrayIndex;
	wds.descriptorCount = static_cast<uint32_t>(m_DescriptorBufferInfo[index][bindingIndex].size());
	wds.descriptorType = BufferViewTypeToVkDescriptorType(ref_cast<BufferView>(descriptorBufferInfos[0].bufferView)->GetCreateInfo().type);
	wds.pImageInfo = nullptr;
	wds.pBufferInfo = m_DescriptorBufferInfo[index][bindingIndex].data();
	wds.pTexelBufferView = nullptr;

	m_WriteDescriptorSets.push_back(wds);
}

void DescriptorSet::AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	for (auto& descriptorImageInfos : descriptorImageInfos)
	{
		m_DescriptorImageInfo[index][bindingIndex].push_back({
			ref_cast<Sampler>(descriptorImageInfos.sampler)->m_Sampler,
			ref_cast<ImageView>(descriptorImageInfos.imageView)->m_ImageView,
			static_cast<VkImageLayout>(descriptorImageInfos.imageLayout)
			});
	}

	auto& usage = ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->GetCreateInfo().usage;

	VkWriteDescriptorSet wds;
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = nullptr;
	wds.dstSet = m_DescriptorSets[index];
	wds.dstBinding = bindingIndex;
	wds.dstArrayElement = desriptorArrayIndex;
	wds.descriptorCount = static_cast<uint32_t>(m_DescriptorImageInfo[index][bindingIndex].size());
	wds.descriptorType = ImageUsageToVkDescriptorType(usage, (descriptorImageInfos[0].sampler != nullptr), (descriptorImageInfos[0].imageView != nullptr));
	wds.pImageInfo = m_DescriptorImageInfo[index][bindingIndex].data();
	wds.pBufferInfo = nullptr;
	wds.pTexelBufferView = nullptr;

	m_WriteDescriptorSets.push_back(wds);
}

void DescriptorSet::Update()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(m_WriteDescriptorSets.size()), m_WriteDescriptorSets.data(), 0, nullptr);
}

VkDescriptorType DescriptorSet::BufferViewTypeToVkDescriptorType(crossplatform::BufferView::Type type)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (type)
	{
	case BufferView::Type::UNIFORM_TEXEL:
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case BufferView::Type::STORAGE_TEXEL:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case BufferView::Type::UNIFORM:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case BufferView::Type::STORAGE:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case BufferView::Type::INDEX:
	case BufferView::Type::VERTEX:
	case BufferView::Type::TRANSFORM_FEEDBACK:
	default:
		return VkDescriptorType(0);
	}
}

VkDescriptorType DescriptorSet::ImageUsageToVkDescriptorType(crossplatform::Image::UsageBit usage, bool sampler, bool imageView)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (usage)
	{
		case Image::UsageBit::TRANSFER_SRC_BIT:
		case Image::UsageBit::TRANSFER_DST_BIT:
		case Image::UsageBit::COLOUR_ATTACHMENT_BIT:
		case Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT:
		case Image::UsageBit::TRANSIENT_ATTACHMENT_BIT:
		default:
		{
			if(sampler && imageView)
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			else if (sampler && !imageView)
				return VK_DESCRIPTOR_TYPE_SAMPLER;
			else
				return VkDescriptorType(0);
		}
		case Image::UsageBit::SAMPLED_BIT:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case Image::UsageBit::STORAGE_BIT:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case Image::UsageBit::INPUT_ATTACHMENT_BIT:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}
}