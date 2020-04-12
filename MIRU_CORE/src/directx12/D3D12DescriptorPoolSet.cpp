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

	ID3D12DescriptorHeap* heap;

	m_DescHeapBaseCPUHandles.resize(ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPools.size());
	for (size_t i = 0; i < ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPools.size(); i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			heap = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPools[i][j];
			if (heap)
				m_DescHeapBaseCPUHandles[i][j] = heap->GetCPUDescriptorHandleForHeapStart();
		}
	}

	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT samplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	m_DescriptorRanges.resize(m_CI.pDescriptorPool->GetCreateInfo().maxSets);
	m_DescriptorRangeSampler.resize(m_CI.pDescriptorPool->GetCreateInfo().maxSets);
	m_DescriptorTables.resize(m_CI.pDescriptorPool->GetCreateInfo().maxSets);
	UINT set = 0;
	for (auto& descriptorSetLayout : m_CI.pDescriptorSetLayouts)
	{
		UINT countCBV = 0;
		UINT countSRV = 0;
		UINT countUAV = 0;
		UINT countSampler = 0;

		INT baseBindingSRV = -1;
		INT baseBindingUAV = -1;
		INT baseBindingCBV = -1;
		INT baseBindingSampler = -1;

		for (auto& descriptorSetLayoutBinding : descriptorSetLayout->GetCreateInfo().descriptorSetLayoutBinding)
		{
			switch (descriptorSetLayoutBinding.type)
			{
			case crossplatform::DescriptorType::SAMPLER:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][1].ptr = m_DescHeapBaseCPUHandles[set][1].ptr + countSampler * samplerDescriptorSize;
				countSampler++;
				if (baseBindingSampler == -1) baseBindingSampler = descriptorSetLayoutBinding.binding;
				continue; 
			}
			case crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][1].ptr = m_DescHeapBaseCPUHandles[set][1].ptr + countSampler * samplerDescriptorSize;
				countSRV++;
				countSampler++;
				if (baseBindingSRV == -1) baseBindingSRV = descriptorSetLayoutBinding.binding;
				if (baseBindingSampler == -1) baseBindingSampler = descriptorSetLayoutBinding.binding;
				continue; 
			}
			case crossplatform::DescriptorType::SAMPLED_IMAGE:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				countSRV++;
				if (baseBindingSRV == -1) baseBindingSRV = descriptorSetLayoutBinding.binding;
				continue; 
			}
			case crossplatform::DescriptorType::STORAGE_IMAGE:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				countUAV++;
				if (baseBindingUAV == -1) baseBindingUAV = descriptorSetLayoutBinding.binding;
				continue; 
			}
			case crossplatform::DescriptorType::UNIFORM_TEXEL_BUFFER:
			case crossplatform::DescriptorType::UNIFORM_BUFFER:
			case crossplatform::DescriptorType::UNIFORM_BUFFER_DYNAMIC:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				countCBV++;
				if (baseBindingCBV == -1) baseBindingCBV = descriptorSetLayoutBinding.binding;
				continue;
			}
			case crossplatform::DescriptorType::STORAGE_TEXEL_BUFFER:
			case crossplatform::DescriptorType::STORAGE_BUFFER:
			case crossplatform::DescriptorType::STORAGE_BUFFER_DYNAMIC:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				countUAV++;
				if (baseBindingUAV == -1) baseBindingUAV = descriptorSetLayoutBinding.binding;
				continue;
			}
			case crossplatform::DescriptorType::INPUT_ATTACHMENT:
			{ 
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				countSRV++; 
				if (baseBindingSRV == -1) baseBindingSRV = descriptorSetLayoutBinding.binding;
				continue;
			}
			default:
			{ 				
				m_DescCPUHandles[set][descriptorSetLayoutBinding.binding][0].ptr = m_DescHeapBaseCPUHandles[set][0].ptr + (countCBV + countSRV + countUAV) * cbv_srv_uav_DescriptorSize;
				countSRV++; 
				if (baseBindingSRV == -1) baseBindingSRV = descriptorSetLayoutBinding.binding;
				continue;
			}
			}
		}

		for (size_t i = 0; i < 4; i++)
		{
			D3D12_DESCRIPTOR_RANGE descRange;
			descRange.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(i);
			descRange.NumDescriptors = i == 0 ? countSRV : i == 1 ? countUAV : i == 2 ? countCBV : countSampler;
			descRange.BaseShaderRegister = i == 0 ? baseBindingSRV : i == 1 ? baseBindingUAV : i == 2 ? baseBindingCBV : baseBindingSampler;
			descRange.RegisterSpace = set;
			descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


			if (descRange.NumDescriptors > 0)
			{
				if(descRange.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
					m_DescriptorRangeSampler[set] = descRange;
				else
					m_DescriptorRanges[set].push_back(descRange);
			}
		}

		m_DescriptorTables[set].NumDescriptorRanges = static_cast<UINT>(m_DescriptorRanges[set].size());
		m_DescriptorTables[set].pDescriptorRanges = m_DescriptorRanges[set].data();
		ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorTable = m_DescriptorTables[set];

		if (countSampler > 0)
		{
			m_DescriptorTableSampler.NumDescriptorRanges = 1;
			m_DescriptorTableSampler.pDescriptorRanges = &m_DescriptorRangeSampler[set];
			ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorTableSampler = m_DescriptorTableSampler;
		}

		set++;
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
	//UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto& descriptorBufferInfo : descriptorBufferInfos)
	{
		auto& type = ref_cast<BufferView>(descriptorBufferInfo.bufferView)->GetCreateInfo().type;

		//CBV
		if (type == BufferView::Type::UNIFORM || type == BufferView::Type::UNIFORM_TEXEL)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][0];

			m_Device->CreateConstantBufferView(&ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_CBVDesc, descriptorWriteLocation);
			ref_cast<BufferView>(descriptorBufferInfo.bufferView)->m_CBVDescHandle = descriptorWriteLocation;
		}
		//UAV
		if (type == BufferView::Type::STORAGE || type == BufferView::Type::STORAGE_TEXEL)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][0];

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

	//UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//UINT dsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//UINT samplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	for (auto& descriptorImageInfo : descriptorImageInfos)
	{
		auto& usage = ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->GetCreateInfo().usage;

		//RTV
		if (usage == Image::UsageBit::COLOUR_ATTACHMENT_BIT)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][2];
			m_Device->CreateRenderTargetView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
				&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_RTVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_RTVDescHandle = descriptorWriteLocation;
		}
		//DSV
		else if (usage == Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][3];
			m_Device->CreateDepthStencilView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
				&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_DSVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_DSVDescHandle = descriptorWriteLocation;
		}
		//UAV
		else if (usage == Image::UsageBit::STORAGE_BIT)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][0];
			m_Device->CreateUnorderedAccessView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image, nullptr,
				&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_UAVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_UAVDescHandle = descriptorWriteLocation;
		}
		//SRV
		else
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][0];
			m_Device->CreateShaderResourceView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfo.imageView)->GetCreateInfo().pImage)->m_Image,
				/*&ref_cast<ImageView>(descriptorImageInfo.imageView)->m_SRVDesc*/0, descriptorWriteLocation);
			ref_cast<ImageView>(descriptorImageInfo.imageView)->m_SRVDescHandle = descriptorWriteLocation;
		}

		//Sampler
		if (descriptorImageInfo.sampler)
		{
			descriptorWriteLocation = m_DescCPUHandles[index][bindingIndex][1];
			m_Device->CreateSampler(&ref_cast<Sampler>(descriptorImageInfo.sampler)->m_SamplerDesc, descriptorWriteLocation);
			ref_cast<Sampler>(descriptorImageInfo.sampler)->m_DescHandle = descriptorWriteLocation;
		}
	}
}

void DescriptorSet::Update()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_RootParameters.clear();

	D3D12_ROOT_PARAMETER rootParameter = {};
	for (auto& descriptorTable : m_DescriptorTables)
	{
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = descriptorTable;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_RootParameters.push_back(rootParameter);
	}

	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.DescriptorTable = m_DescriptorTableSampler;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	m_RootParameters.push_back(rootParameter);
}

