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
}

DescriptorPool::~DescriptorPool()
{
	MIRU_CPU_PROFILE_FUNCTION();
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
			countSampler += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingSampler == ~0U)
				baseBindingSampler = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER:
		{
			countSRV++;
			countSampler += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			if (baseBindingSampler == ~0U)
				baseBindingSampler = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::SAMPLED_IMAGE:
		{
			countSRV += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::STORAGE_IMAGE:
		{
			countUAV += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingUAV == ~0U)
				baseBindingUAV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::UNIFORM_TEXEL_BUFFER:
		case crossplatform::DescriptorType::UNIFORM_BUFFER:
		case crossplatform::DescriptorType::UNIFORM_BUFFER_DYNAMIC:
		{
			countCBV += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingCBV == ~0U)
				baseBindingCBV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::STORAGE_TEXEL_BUFFER:
		case crossplatform::DescriptorType::STORAGE_BUFFER:
		case crossplatform::DescriptorType::STORAGE_BUFFER_DYNAMIC:
		{
			countUAV += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingUAV == ~0U)
				baseBindingUAV = descriptorSetLayoutBinding.binding;
			continue;
		}
		case crossplatform::DescriptorType::INPUT_ATTACHMENT:
		{
			countSRV += descriptorSetLayoutBinding.descriptorCount;
			if (baseBindingSRV == ~0U)
				baseBindingSRV = descriptorSetLayoutBinding.binding;
			continue;
		}
		default:
		{
			countSRV += descriptorSetLayoutBinding.descriptorCount;
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

	const Ref<DescriptorPool>& descriptorPool = ref_cast<DescriptorPool>(m_CI.pDescriptorPool);
	DescriptorPool::CreateInfo descriptorPoolCI = descriptorPool->GetCreateInfo();
	
	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT samplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT dsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	MIRU_ASSERT(((m_CI.pDescriptorSetLayouts.size() + descriptorPool->m_AssignedSets) > descriptorPoolCI.maxSets),
		"ERROR: D3D12: Exceeded max descriptor sets for this pool.");

	uint32_t index = 0;
	for (auto& descriptorSetLayouts : m_CI.pDescriptorSetLayouts)
	{
		MIRU_ASSERT(((index >= descriptorPoolCI.maxSets) && index > 0), "ERROR: D3D12: Exceeded max descriptor sets for this pool.");

		uint32_t numDescriptors_Sampler = 0;
		uint32_t numDescriptors_CBV_SRV_UAV = 0;
		uint32_t numDescriptors_RTV = 0;
		uint32_t numDescriptors_DSV = 0;
		for (auto& descriptorSetLayoutBinding : descriptorSetLayouts->GetCreateInfo().descriptorSetLayoutBinding)
		{
			if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::SAMPLER)
			{
				numDescriptors_Sampler += descriptorSetLayoutBinding.descriptorCount;
			}
			else if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER)
			{
				numDescriptors_Sampler += descriptorSetLayoutBinding.descriptorCount;
				numDescriptors_CBV_SRV_UAV += descriptorSetLayoutBinding.descriptorCount;
			}
			else if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::D3D12_RENDER_TARGET_VIEW)
			{
				numDescriptors_RTV += descriptorSetLayoutBinding.descriptorCount;
			}
			else if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::D3D12_DEPTH_STENCIL_VIEW)
			{
				numDescriptors_DSV += descriptorSetLayoutBinding.descriptorCount;
			}
			else
				numDescriptors_CBV_SRV_UAV += descriptorSetLayoutBinding.descriptorCount;
		}

		m_DescriptorHeaps.push_back({});
		m_DescriptorHeapDescs.push_back({});

		m_DescriptorHeapDescs[index][0].Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_DescriptorHeapDescs[index][0].NumDescriptors = numDescriptors_CBV_SRV_UAV;
		m_DescriptorHeapDescs[index][0].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_DescriptorHeapDescs[index][0].NodeMask = 0;

		m_DescriptorHeapDescs[index][1].Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		m_DescriptorHeapDescs[index][1].NumDescriptors = numDescriptors_Sampler;
		m_DescriptorHeapDescs[index][1].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_DescriptorHeapDescs[index][1].NodeMask = 0;

		m_DescriptorHeapDescs[index][2].Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		m_DescriptorHeapDescs[index][2].NumDescriptors = numDescriptors_RTV;
		m_DescriptorHeapDescs[index][2].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_DescriptorHeapDescs[index][2].NodeMask = 0;

		m_DescriptorHeapDescs[index][3].Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		m_DescriptorHeapDescs[index][3].NumDescriptors = numDescriptors_DSV;
		m_DescriptorHeapDescs[index][3].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_DescriptorHeapDescs[index][3].NodeMask = 0;

		if (numDescriptors_CBV_SRV_UAV > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorHeapDescs[index][0], IID_PPV_ARGS(&m_DescriptorHeaps[index][0])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_CBV_SRV_UAV.");
			D3D12SetName(m_DescriptorHeaps[index][0], (descriptorPoolCI.debugName + std::string(" : HEAP_TYPE_CBV_SRV_UAV")).c_str());
		}
		if (numDescriptors_Sampler > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorHeapDescs[index][1], IID_PPV_ARGS(&m_DescriptorHeaps[index][1])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_SAMPLER.");
			D3D12SetName(m_DescriptorHeaps[index][1], (descriptorPoolCI.debugName + std::string(" : HEAP_TYPE_SAMPLER")).c_str());
		}
		if (numDescriptors_RTV > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorHeapDescs[index][2], IID_PPV_ARGS(&m_DescriptorHeaps[index][2])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_RTV.");
			D3D12SetName(m_DescriptorHeaps[index][2], (descriptorPoolCI.debugName + std::string(" : HEAP_TYPE_RTV")).c_str());
		}
		if (numDescriptors_DSV > 0)
		{
			MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorHeapDescs[index][3], IID_PPV_ARGS(&m_DescriptorHeaps[index][3])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_DSV.");
			D3D12SetName(m_DescriptorHeaps[index][3], (descriptorPoolCI.debugName + std::string(" : HEAP_TYPE_DSV")).c_str());
		}

		uint32_t binding_Sampler = 0;
		uint32_t binding_CBV_SRV_UAV = 0;
		uint32_t binding_RTV = 0;
		uint32_t binding_DSV = 0;
		for (auto& descriptorSetLayoutBinding : descriptorSetLayouts->GetCreateInfo().descriptorSetLayoutBinding)
		{
			uint32_t descBinding = descriptorSetLayoutBinding.binding;
			if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::SAMPLER)
			{
				m_DescCPUHandles[index][descBinding][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].ptr =
					m_DescriptorHeaps[index][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->GetCPUDescriptorHandleForHeapStart().ptr
					+ binding_Sampler * samplerDescriptorSize;
				binding_Sampler++;
			}
			else if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER)
			{
				m_DescCPUHandles[index][descBinding][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].ptr =
					m_DescriptorHeaps[index][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart().ptr
					+ binding_CBV_SRV_UAV * cbv_srv_uav_DescriptorSize;
				binding_CBV_SRV_UAV++;

				m_DescCPUHandles[index][descBinding][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].ptr =
					m_DescriptorHeaps[index][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->GetCPUDescriptorHandleForHeapStart().ptr
					+ binding_Sampler * samplerDescriptorSize;
				binding_Sampler++;
			}
			else if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::D3D12_RENDER_TARGET_VIEW)
			{
				m_DescCPUHandles[index][descBinding][D3D12_DESCRIPTOR_HEAP_TYPE_RTV].ptr =
					m_DescriptorHeaps[index][D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart().ptr
					+ binding_RTV * rtvDescriptorSize;
				binding_RTV++;
			}
			else if (descriptorSetLayoutBinding.type == crossplatform::DescriptorType::D3D12_DEPTH_STENCIL_VIEW)
			{
				m_DescCPUHandles[index][descBinding][D3D12_DESCRIPTOR_HEAP_TYPE_DSV].ptr =
					m_DescriptorHeaps[index][D3D12_DESCRIPTOR_HEAP_TYPE_DSV]->GetCPUDescriptorHandleForHeapStart().ptr
					+ binding_DSV * dsvDescriptorSize;
				binding_DSV++;
			}
			else
			{
				m_DescCPUHandles[index][descBinding][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].ptr =
					m_DescriptorHeaps[index][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart().ptr
					+ binding_CBV_SRV_UAV * cbv_srv_uav_DescriptorSize;
				binding_CBV_SRV_UAV++;
			}
		}
		index++;
	}
	descriptorPool->m_AssignedSets = m_CI.pDescriptorSetLayouts.size();
}

DescriptorSet::~DescriptorSet()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto& descriptorHeap : m_DescriptorHeaps)
	{
		SAFE_RELEASE(descriptorHeap[0]);
		SAFE_RELEASE(descriptorHeap[1]);
		SAFE_RELEASE(descriptorHeap[2]);
		SAFE_RELEASE(descriptorHeap[3]);
	}
}

void DescriptorSet::AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex)
{
	MIRU_CPU_PROFILE_FUNCTION();

	CHECK_VALID_INDEX_RETURN(index);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorWriteLocation;

	for (auto& descriptorBufferInfo : descriptorBufferInfos)
	{
		auto& type = ref_cast<BufferView>(descriptorBufferInfo.bufferView)->GetCreateInfo().type;

		//CBV
		if (type == BufferView::Type::UNIFORM || type == BufferView::Type::UNIFORM_TEXEL)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];

			m_Device->CreateConstantBufferView(&ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_CBVDesc, descriptorWriteLocation);
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_CBVDescHandle = descriptorWriteLocation;
		}
		//UAV
		if (type == BufferView::Type::STORAGE || type == BufferView::Type::STORAGE_TEXEL)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];

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

	for (auto& descriptorImageInfo : descriptorImageInfos)
	{
		crossplatform::DescriptorType type = m_CI.pDescriptorSetLayouts[index]->GetCreateInfo().descriptorSetLayoutBinding[bindingIndex].type;

		//Image View
		if (descriptorImageInfo.imageView)
		{
			//RTV
			if (type == crossplatform::DescriptorType::D3D12_RENDER_TARGET_VIEW)
			{
				descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
				m_Device->CreateRenderTargetView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
					&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_RTVDesc, descriptorWriteLocation);
				ref_cast<ImageView>(descriptorImageInfo.imageView)->m_RTVDescHandle = descriptorWriteLocation;
			}
			//DSV
			if (type == crossplatform::DescriptorType::D3D12_DEPTH_STENCIL_VIEW)
			{
				descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_DSV];
				m_Device->CreateDepthStencilView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
					&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_DSVDesc, descriptorWriteLocation);
				ref_cast<ImageView>(descriptorImageInfo.imageView)->m_DSVDescHandle = descriptorWriteLocation;
			}
			//UAV
			if (type == crossplatform::DescriptorType::STORAGE_IMAGE || type == crossplatform::DescriptorType::STORAGE_TEXEL_BUFFER)
			{
				descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
				m_Device->CreateUnorderedAccessView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image, nullptr,
					&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_UAVDesc, descriptorWriteLocation);
				ref_cast<ImageView>(descriptorImageInfo.imageView)->m_UAVDescHandle = descriptorWriteLocation;
			}
			//SRV
			if(type == crossplatform::DescriptorType::SAMPLED_IMAGE || type == crossplatform::DescriptorType::UNIFORM_TEXEL_BUFFER
				|| type == crossplatform::DescriptorType::INPUT_ATTACHMENT || type == crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER)
			{
				descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
				m_Device->CreateShaderResourceView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
					&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_SRVDesc, descriptorWriteLocation);
				ref_cast<ImageView>(descriptorImageInfo.imageView)->m_SRVDescHandle = descriptorWriteLocation;
			}
		}

		//Sampler
		if (descriptorImageInfo.sampler)
		{
			//SAMPLER
			if (type == crossplatform::DescriptorType::SAMPLER)
			{
				descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER];
				m_Device->CreateSampler(&ref_cast<Sampler>(descriptorImageInfo.sampler)->m_SamplerDesc, descriptorWriteLocation);
				ref_cast<Sampler>(descriptorImageInfo.sampler)->m_DescHandle = descriptorWriteLocation;
			}
		}
	}
}

void DescriptorSet::Update()
{
	MIRU_CPU_PROFILE_FUNCTION();

}

