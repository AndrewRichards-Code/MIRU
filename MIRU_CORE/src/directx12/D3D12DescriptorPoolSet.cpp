#include "common.h"
#include "D3D12DescriptorPoolSet.h"
#include "D3D12Buffer.h"
#include "D3D12Image.h"

using namespace miru;
using namespace d3d12;

//DescriptorPool
DescriptorPool::DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	uint32_t numDescriptors_Sampler = 0;
	uint32_t numDescriptors_CBV_SRV_UAV = 0;

	for (auto& poolSize : m_CI.poolSizes)
	{
		if (poolSize.type == crossplatform::DescriptorType::SAMPLER && poolSize.type == crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER)
			numDescriptors_Sampler += poolSize.descriptorCount;
		else
			numDescriptors_CBV_SRV_UAV += poolSize.descriptorCount;
	}

	if (numDescriptors_Sampler > 0)
	{
		m_DescriptorPoolDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		m_DescriptorPoolDesc.NumDescriptors = numDescriptors_Sampler;
		m_DescriptorPoolDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_DescriptorPoolDesc.NodeMask = 0;

		MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDesc, IID_PPV_ARGS(&m_DescriptorPool[0])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_SAMPLER.");
		D3D12SetName(m_DescriptorPool[0], (m_CI.debugName + std::string(" : HEAP_TYPE_SAMPLER")).c_str());
	}
	if (numDescriptors_CBV_SRV_UAV > 0)
	{
		m_DescriptorPoolDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_DescriptorPoolDesc.NumDescriptors = numDescriptors_CBV_SRV_UAV;
		m_DescriptorPoolDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_DescriptorPoolDesc.NodeMask = 0;

		MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDesc, IID_PPV_ARGS(&m_DescriptorPool[1])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_CBV_SRV_UAV.");
		D3D12SetName(m_DescriptorPool[1], (m_CI.debugName + std::string(" : HEAP_TYPE_CBV_SRV_UAV")).c_str());
	}
}
DescriptorPool::~DescriptorPool()
{
	SAFE_RELEASE(m_DescriptorPool[0]);
	SAFE_RELEASE(m_DescriptorPool[1]);
}

//DescriptorSetLayout
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_DescriptorSetRanges.

	/*for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayoutBinding)
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

	MIRU_ASSERT(vkCreateDescriptorSetLayout(m_Device, &m_DescriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout), "ERROR: VULKAN: Failed to create m_DescriptorSetLayout.");*/
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

//DescriptorSet
DescriptorSet::DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(std::dynamic_pointer_cast<DescriptorPool>(pCreateInfo->pDescriptorPool)->GetCreateInfo().device))
{
	m_CI = *pCreateInfo;

	for (auto& descriptorSetLayout : m_CI.pDescriptorSetLayouts)
		m_DescriptorSetLayouts.push_back(std::dynamic_pointer_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorSetLayout);

	m_DescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	m_DescriptorSetAI.pNext = nullptr;
	m_DescriptorSetAI.descriptorPool = std::dynamic_pointer_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool;
	m_DescriptorSetAI.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
	m_DescriptorSetAI.pSetLayouts = m_DescriptorSetLayouts.data();

	m_DescriptorSets.resize(m_DescriptorSetLayouts.size());

	MIRU_ASSERT(vkAllocateDescriptorSets(m_Device, &m_DescriptorSetAI, m_DescriptorSets.data()), "ERROR: VULKAN: Failed to create m_DescriptorSetLayout.");
}

DescriptorSet::~DescriptorSet()
{
	vkFreeDescriptorSets(m_Device, m_DescriptorSetAI.descriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());
}

void DescriptorSet::AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex)
{
	CHECK_VALID_INDEX_RETURN(index);

	for (auto& descriptorBufferInfo : descriptorBufferInfos)
	{
		m_DescriptorBufferInfo[index][bindingIndex].push_back({
			std::dynamic_pointer_cast<Buffer>(std::dynamic_pointer_cast<BufferView>(descriptorBufferInfo.bufferView)->GetCreateInfo().pBuffer)->m_Buffer,
			std::dynamic_pointer_cast<BufferView>(descriptorBufferInfo.bufferView)->m_BufferViewCI.offset,
			std::dynamic_pointer_cast<BufferView>(descriptorBufferInfo.bufferView)->m_BufferViewCI.range
			});
	}

	VkWriteDescriptorSet wds;
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = nullptr;
	wds.dstSet = m_DescriptorSets[index];
	wds.dstBinding = bindingIndex;
	wds.dstArrayElement = desriptorArrayIndex;
	wds.descriptorCount = static_cast<uint32_t>(m_DescriptorBufferInfo[index][bindingIndex].size());
	wds.descriptorType = BufferViewTypeToVkDescriptorType(std::dynamic_pointer_cast<BufferView>(descriptorBufferInfos[0].bufferView)->GetCreateInfo().type);
	wds.pImageInfo = nullptr;
	wds.pBufferInfo = m_DescriptorBufferInfo[index][bindingIndex].data();
	wds.pTexelBufferView = nullptr;

	m_WriteDescriptorSets.push_back(wds);
}

void DescriptorSet::AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex)
{
	CHECK_VALID_INDEX_RETURN(index);

	for (auto& descriptorImageInfos : descriptorImageInfos)
	{
		m_DescriptorImageInfo[index][bindingIndex].push_back({
			VK_NULL_HANDLE,//std::dynamic_pointer_cast<Sampler>(descriptorImageInfos.pSampler)->m_Sampler,
			std::dynamic_pointer_cast<ImageView>(descriptorImageInfos.imageView)->m_ImageView,
			static_cast<VkImageLayout>(descriptorImageInfos.imageLayout)
			});
	}

	auto& usage = std::dynamic_pointer_cast<Image>(std::dynamic_pointer_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->GetCreateInfo().usage;

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
	vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(m_WriteDescriptorSets.size()), m_WriteDescriptorSets.data(), 0, nullptr);
}

VkDescriptorType DescriptorSet::BufferViewTypeToVkDescriptorType(crossplatform::BufferView::Type type)
{
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
	switch (usage)
	{
		case Image::UsageBit::TRANSFER_SRC_BIT:
		case Image::UsageBit::TRANSFER_DST_BIT:
		case Image::UsageBit::COLOR_ATTACHMENT_BIT:
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