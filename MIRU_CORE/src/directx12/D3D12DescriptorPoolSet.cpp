#include "miru_core_common.h"
#include "D3D12DescriptorPoolSet.h"
#include "D3D12Buffer.h"
#include "D3D12Image.h"

using namespace miru;
using namespace d3d12;

//DescriptorPool
DescriptorPool::DescriptorPool(DescriptorPool::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	uint32_t numDescriptors_Sampler = 0;
	uint32_t numDescriptors_CBV_SRV_UAV = 0;
	uint32_t numDescriptors_RTV = 0;
	uint32_t numDescriptors_DSV = 0;

	for (auto& poolSize : m_CI.poolSizes)
	{
		if (poolSize.type == crossplatform::DescriptorType::SAMPLER)
			numDescriptors_Sampler += poolSize.descriptorCount;
		else if (poolSize.type == crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER)
		{
			numDescriptors_Sampler += poolSize.descriptorCount;
			numDescriptors_CBV_SRV_UAV += poolSize.descriptorCount;
		}
		else if (poolSize.type == crossplatform::DescriptorType::D3D12_RENDER_TARGET_VIEW)
			numDescriptors_RTV += poolSize.descriptorCount;
		else if (poolSize.type == crossplatform::DescriptorType::D3D12_DEPTH_STENCIL_VIEW)
			numDescriptors_DSV += poolSize.descriptorCount;
		else
			numDescriptors_CBV_SRV_UAV += poolSize.descriptorCount;
	}

	m_DescriptorPoolDescs[0].Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_DescriptorPoolDescs[0].NumDescriptors = numDescriptors_CBV_SRV_UAV;
	m_DescriptorPoolDescs[0].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_DescriptorPoolDescs[0].NodeMask = 0;
						
	m_DescriptorPoolDescs[1].Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	m_DescriptorPoolDescs[1].NumDescriptors = numDescriptors_Sampler;
	m_DescriptorPoolDescs[1].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_DescriptorPoolDescs[1].NodeMask = 0;
						
	m_DescriptorPoolDescs[2].Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_DescriptorPoolDescs[2].NumDescriptors = numDescriptors_RTV;
	m_DescriptorPoolDescs[2].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_DescriptorPoolDescs[2].NodeMask = 0;
						
	m_DescriptorPoolDescs[3].Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_DescriptorPoolDescs[3].NumDescriptors = numDescriptors_DSV;
	m_DescriptorPoolDescs[3].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_DescriptorPoolDescs[3].NodeMask = 0;

	m_DescriptorPools.resize(m_CI.maxSets);
	for (size_t i = 0; i < m_CI.maxSets; i++)
	{
		if (numDescriptors_CBV_SRV_UAV > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDescs[0], IID_PPV_ARGS(&m_DescriptorPools[i][0])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_CBV_SRV_UAV.");
			D3D12SetName(m_DescriptorPools[i][0], (m_CI.debugName + std::string(" : HEAP_TYPE_CBV_SRV_UAV")).c_str());
		}
		if (numDescriptors_Sampler > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDescs[1], IID_PPV_ARGS(&m_DescriptorPools[i][1])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_SAMPLER.");
			D3D12SetName(m_DescriptorPools[i][1], (m_CI.debugName + std::string(" : HEAP_TYPE_SAMPLER")).c_str());
		}
		if (numDescriptors_RTV > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDescs[2], IID_PPV_ARGS(&m_DescriptorPools[i][2])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_RTV.");
			D3D12SetName(m_DescriptorPools[i][2], (m_CI.debugName + std::string(" : HEAP_TYPE_RTV")).c_str());
		}
		if (numDescriptors_DSV > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDescs[3], IID_PPV_ARGS(&m_DescriptorPools[i][3])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_DSV.");
			D3D12SetName(m_DescriptorPools[i][3], (m_CI.debugName + std::string(" : HEAP_TYPE_DSV")).c_str());
		}
	}

	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT samplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT dsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	for (uint32_t set = 0; set < m_CI.maxSets; set++)
	{
		for (uint32_t binding = 0; binding < numDescriptors_CBV_SRV_UAV; binding++)
		{
			m_DescCPUHandles[set][binding][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].ptr =
				m_DescriptorPools[set][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart().ptr
				+ binding * cbv_srv_uav_DescriptorSize;
		}
		for (uint32_t binding = 0; binding < numDescriptors_Sampler; binding++)
		{
			m_DescCPUHandles[set][binding][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].ptr =
				m_DescriptorPools[set][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->GetCPUDescriptorHandleForHeapStart().ptr
				+ binding * samplerDescriptorSize;
		}
		for (uint32_t binding = 0; binding < numDescriptors_RTV; binding++)
		{
			m_DescCPUHandles[set][binding][D3D12_DESCRIPTOR_HEAP_TYPE_RTV].ptr =
				m_DescriptorPools[set][D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart().ptr
				+ binding * rtvDescriptorSize;
		}
		for (uint32_t binding = 0; binding < numDescriptors_DSV; binding++)
		{
			m_DescCPUHandles[set][binding][D3D12_DESCRIPTOR_HEAP_TYPE_DSV].ptr =
				m_DescriptorPools[set][D3D12_DESCRIPTOR_HEAP_TYPE_DSV]->GetCPUDescriptorHandleForHeapStart().ptr
				+ binding * dsvDescriptorSize;
		}
	}
}

DescriptorPool::~DescriptorPool()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (size_t i = 0; i < m_CI.maxSets; i++)
	{
		SAFE_RELEASE(m_DescriptorPools[i][0]);
		SAFE_RELEASE(m_DescriptorPools[i][1]);
		SAFE_RELEASE(m_DescriptorPools[i][2]);
		SAFE_RELEASE(m_DescriptorPools[i][3]);
	}
}

//DescriptorSetLayout
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	UINT countCBV = 0;
	UINT countSRV = 0;
	UINT countUAV = 0;
	UINT countSampler = 0;

	INT baseBindingSRV = ~0U;
	INT baseBindingUAV = ~0U;
	INT baseBindingCBV = ~0U;
	INT baseBindingSampler = ~0U;

	for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayoutBinding)
	{
		switch (descriptorSetLayoutBinding.type)
		{
		case crossplatform::DescriptorType::SAMPLER:
		{
			countSampler++;
			if (baseBindingSampler == ~0U)
				baseBindingSampler = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER:
		{
			countSRV++;
			countSampler++;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			if (baseBindingSampler == ~0U)
				baseBindingSampler = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::SAMPLED_IMAGE:
		{
			countSRV++;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::STORAGE_IMAGE:
		{
			countUAV++;
			if (baseBindingUAV == ~0U)
				baseBindingUAV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::UNIFORM_TEXEL_BUFFER:
		case crossplatform::DescriptorType::UNIFORM_BUFFER:
		case crossplatform::DescriptorType::UNIFORM_BUFFER_DYNAMIC:
		{
			countCBV++;
			if (baseBindingCBV == ~0U)
				baseBindingCBV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::STORAGE_TEXEL_BUFFER:
		case crossplatform::DescriptorType::STORAGE_BUFFER:
		case crossplatform::DescriptorType::STORAGE_BUFFER_DYNAMIC:
		{
			countUAV++;
			if (baseBindingUAV == ~0U)
				baseBindingUAV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::INPUT_ATTACHMENT:
		{
			countSRV++;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			continue;
		}
		default:
		{
			countSRV++;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			continue;
		}
		}
	}

	for (size_t i = 0; i < m_DescriptorRanges.size(); i++)
	{
		m_DescriptorRanges[i].RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(i);
		m_DescriptorRanges[i].NumDescriptors = i == 0 ? countSRV : i == 1 ? countUAV : i == 2 ? countCBV : countSampler;
		m_DescriptorRanges[i].BaseShaderRegister = i == 0 ? baseBindingSRV : i == 1 ? baseBindingUAV : i == 2 ? baseBindingCBV : baseBindingSampler;
		m_DescriptorRanges[i].RegisterSpace = ~0U;
		m_DescriptorRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

//DescriptorSet
DescriptorSet::DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(ref_cast<DescriptorPool>(pCreateInfo->pDescriptorPool)->GetCreateInfo().device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	uint32_t index = 0;
	for (auto& descriptorSetLayout : m_CI.pDescriptorSetLayouts)
	{
		uint32_t binding = 0;
		for (auto& descriptorSetLayoutBinding : descriptorSetLayout->GetCreateInfo().descriptorSetLayoutBinding)
		{
			if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::SAMPLER
				|| descriptorSetLayoutBinding.type == crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER)
			{
				uint32_t currentBinding = descriptorSetLayoutBinding.binding;
				m_SamplerBindings[index][currentBinding] = binding;
				binding++;
			}
		}
		index++;
	}

}

DescriptorSet::~DescriptorSet()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

void DescriptorSet::AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorWriteLocation;
	auto descCPUHandles = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescCPUHandles;

	for (auto& descriptorBufferInfo : descriptorBufferInfos)
	{
		auto& type = ref_cast<BufferView>(descriptorBufferInfo.bufferView)->GetCreateInfo().type;

		//CBV
		if (type == BufferView::Type::UNIFORM || type == BufferView::Type::UNIFORM_TEXEL)
		{
			descriptorWriteLocation = descCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];

			m_Device->CreateConstantBufferView(&ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_CBVDesc, descriptorWriteLocation);
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_CBVDescHandle = descriptorWriteLocation;
		}
		//UAV
		if (type == BufferView::Type::STORAGE || type == BufferView::Type::STORAGE_TEXEL)
		{
			descriptorWriteLocation = descCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];

			m_Device->CreateUnorderedAccessView(ref_cast<Buffer>(descriptorBufferInfo.bufferView)->m_Buffer, nullptr,
				&ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_UAVDesc, descriptorWriteLocation);
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_UAVDescHandle = descriptorWriteLocation;
		}
	}
}

void DescriptorSet::AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorWriteLocation;
	auto descCPUHandles = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescCPUHandles;

	for (auto& descriptorImageInfo : descriptorImageInfos)
	{
		auto& usage = ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->GetCreateInfo().usage;

		//RTV
		if (usage == Image::UsageBit::COLOUR_ATTACHMENT_BIT)
		{
			descriptorWriteLocation = descCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
			m_Device->CreateRenderTargetView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
				&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_RTVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_RTVDescHandle = descriptorWriteLocation;
		}
		//DSV
		else if (usage == Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			descriptorWriteLocation = descCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
			m_Device->CreateDepthStencilView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
				&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_DSVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_DSVDescHandle = descriptorWriteLocation;
		}
		//UAV
		else if (usage == Image::UsageBit::STORAGE_BIT)
		{
			descriptorWriteLocation = descCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
			m_Device->CreateUnorderedAccessView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image, nullptr,
				&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_UAVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_UAVDescHandle = descriptorWriteLocation;
		}
		//SRV
		else
		{
			descriptorWriteLocation = descCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
			m_Device->CreateShaderResourceView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
				/*&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_SRVDesc*/0, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_SRVDescHandle = descriptorWriteLocation;
		}

		//Sampler
		if (descriptorImageInfo.sampler)
		{
			uint32_t samplerBinding = m_SamplerBindings[index][bindingIndex];
			descriptorWriteLocation = descCPUHandles[index][samplerBinding][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER];
			m_Device->CreateSampler(&ref_cast<Sampler>(descriptorImageInfo.sampler)->m_SamplerDesc, descriptorWriteLocation);
			ref_cast<Sampler>(descriptorImageInfo.sampler)->m_DescHandle = descriptorWriteLocation;
		}
	}
}

void DescriptorSet::Update()
{
	MIRU_CPU_PROFILE_FUNCTION();

}

