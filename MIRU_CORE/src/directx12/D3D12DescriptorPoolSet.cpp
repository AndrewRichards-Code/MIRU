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

	m_DescriptorPoolDesc[0].Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_DescriptorPoolDesc[0].NumDescriptors = numDescriptors_CBV_SRV_UAV;
	m_DescriptorPoolDesc[0].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_DescriptorPoolDesc[0].NodeMask = 0;
	
	m_DescriptorPoolDesc[1].Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	m_DescriptorPoolDesc[1].NumDescriptors = numDescriptors_Sampler;
	m_DescriptorPoolDesc[1].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_DescriptorPoolDesc[1].NodeMask = 0;
	
	m_DescriptorPoolDesc[2].Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_DescriptorPoolDesc[2].NumDescriptors = numDescriptors_RTV;
	m_DescriptorPoolDesc[2].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_DescriptorPoolDesc[2].NodeMask = 0;
	
	m_DescriptorPoolDesc[3].Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_DescriptorPoolDesc[3].NumDescriptors = numDescriptors_DSV;
	m_DescriptorPoolDesc[3].Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_DescriptorPoolDesc[3].NodeMask = 0;

	if (numDescriptors_CBV_SRV_UAV > 0)
	{
		MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDesc[0], IID_PPV_ARGS(&m_DescriptorPool[0])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_CBV_SRV_UAV.");
		D3D12SetName(m_DescriptorPool[0], (m_CI.debugName + std::string(" : HEAP_TYPE_CBV_SRV_UAV")).c_str());
	}
	if (numDescriptors_Sampler > 0)
	{
		MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDesc[1], IID_PPV_ARGS(&m_DescriptorPool[1])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_SAMPLER.");
		D3D12SetName(m_DescriptorPool[1], (m_CI.debugName + std::string(" : HEAP_TYPE_SAMPLER")).c_str());
	}
	if (numDescriptors_RTV > 0)
	{
		MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDesc[2], IID_PPV_ARGS(&m_DescriptorPool[2])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_RTV.");
		D3D12SetName(m_DescriptorPool[2], (m_CI.debugName + std::string(" : HEAP_TYPE_RTV")).c_str());
	}
	if (numDescriptors_DSV > 0)
	{
		MIRU_ASSERT(m_Device->CreateDescriptorHeap(&m_DescriptorPoolDesc[3], IID_PPV_ARGS(&m_DescriptorPool[3])), "ERROR: D3D12: Failed to create DescriptorPool for HEAP_TYPE_DSV.");
		D3D12SetName(m_DescriptorPool[3], (m_CI.debugName + std::string(" : HEAP_TYPE_DSV")).c_str());
	}
}
DescriptorPool::~DescriptorPool()
{
	SAFE_RELEASE(m_DescriptorPool[0]);
	SAFE_RELEASE(m_DescriptorPool[1]);
	SAFE_RELEASE(m_DescriptorPool[2]);
	SAFE_RELEASE(m_DescriptorPool[3]);
}

//DescriptorSetLayout
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	UINT set = 0;
	for (auto& descriptorSetLayoutBinding : m_CI.descriptorSetLayoutBinding)
	{
		D3D12_DESCRIPTOR_RANGE descRanges;
		D3D12_DESCRIPTOR_RANGE_TYPE type;
		switch (descriptorSetLayoutBinding.type)
		{
		case crossplatform::DescriptorType::SAMPLER:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
		case crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
		case crossplatform::DescriptorType::SAMPLED_IMAGE:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
		case crossplatform::DescriptorType::STORAGE_IMAGE:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
		case crossplatform::DescriptorType::UNIFORM_TEXEL_BUFFER:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
		case crossplatform::DescriptorType::STORAGE_TEXEL_BUFFER:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
		case crossplatform::DescriptorType::UNIFORM_BUFFER:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
		case crossplatform::DescriptorType::STORAGE_BUFFER:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
		case crossplatform::DescriptorType::UNIFORM_BUFFER_DYNAMIC:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
		case crossplatform::DescriptorType::STORAGE_BUFFER_DYNAMIC:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
		case crossplatform::DescriptorType::INPUT_ATTACHMENT:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
		default:
			type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
		}
		descRanges.RangeType = type;
		descRanges.NumDescriptors = descriptorSetLayoutBinding.descriptorCount;
		descRanges.BaseShaderRegister = descriptorSetLayoutBinding.binding;
		descRanges.RegisterSpace = 0;
		descRanges.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		m_DescriptorRanges.push_back(descRanges);
	}

	m_DescriptorTable.pDescriptorRanges = m_DescriptorRanges.data();
	m_DescriptorTable.NumDescriptorRanges = static_cast<UINT>(m_DescriptorRanges.size());
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	m_DescriptorRanges.clear();
}

//DescriptorSet
DescriptorSet::DescriptorSet(DescriptorSet::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(ref_cast<DescriptorPool>(pCreateInfo->pDescriptorPool)->GetCreateInfo().device))
{
	m_CI = *pCreateInfo;

	ID3D12DescriptorHeap* heap;

	heap = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool[0];
	if (heap)
		m_CBV_SRV_UAV_DescHeapCPUHandle = heap->GetCPUDescriptorHandleForHeapStart();

	heap = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool[1];
	if (heap)
		m_Sampler_DescHeapCPUHandle = heap->GetCPUDescriptorHandleForHeapStart();

	heap = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool[2];
	if (heap)
		m_RTV_DescHeapCPUHandle = heap->GetCPUDescriptorHandleForHeapStart();

	heap = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool[3];
	if (heap)
		m_DSV_DescHeapCPUHandle = heap->GetCPUDescriptorHandleForHeapStart();

	/*for (auto& descriptorSetLayout : m_CI.pDescriptorSetLayouts)
		m_DescriptorSetLayouts.push_back(ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorSetLayout);

	m_DescriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	m_DescriptorSetAI.pNext = nullptr;
	m_DescriptorSetAI.descriptorPool = ref_cast<DescriptorPool>(m_CI.pDescriptorPool)->m_DescriptorPool;
	m_DescriptorSetAI.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
	m_DescriptorSetAI.pSetLayouts = m_DescriptorSetLayouts.data();

	m_DescriptorSets.resize(m_DescriptorSetLayouts.size());

	MIRU_ASSERT(vkAllocateDescriptorSets(m_Device, &m_DescriptorSetAI, m_DescriptorSets.data()), "ERROR: VULKAN: Failed to create m_DescriptorSetLayout.");*/
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::AddBuffer(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorBufferInfo>& descriptorBufferInfos, uint32_t desriptorArrayIndex)
{
	CHECK_VALID_INDEX_RETURN(index);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorWriteLocation;
	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto& type = ref_cast<BufferView>(descriptorBufferInfos[0].bufferView)->GetCreateInfo().type;

	//CBV
	if (type == BufferView::Type::UNIFORM || type == BufferView::Type::UNIFORM_TEXEL)
	{
		descriptorWriteLocation = m_CBV_SRV_UAV_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (cbv_srv_uav_DescriptorSize * index);

		m_Device->CreateConstantBufferView(&ref_cast<BufferView>(descriptorBufferInfos[0].bufferView)->m_CBVDesc, descriptorWriteLocation);
	}
	//UAV
	if (type == BufferView::Type::STORAGE || type == BufferView::Type::STORAGE_TEXEL)
	{
		descriptorWriteLocation = m_CBV_SRV_UAV_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (cbv_srv_uav_DescriptorSize * index);

		m_Device->CreateUnorderedAccessView(ref_cast<Buffer>(descriptorBufferInfos[0].bufferView)->m_Buffer, nullptr,
			&ref_cast<BufferView>(descriptorBufferInfos[0].bufferView)->m_UAVDesc, descriptorWriteLocation);
	}
}

void DescriptorSet::AddImage(uint32_t index, uint32_t bindingIndex, const std::vector<DescriptorImageInfo>& descriptorImageInfos, uint32_t desriptorArrayIndex)
{
	CHECK_VALID_INDEX_RETURN(index);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorWriteLocation;

	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT dsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT samplerDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	auto& usage = ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->GetCreateInfo().usage;

	//RTV
	if (usage == Image::UsageBit::COLOR_ATTACHMENT_BIT)
	{
		descriptorWriteLocation = m_RTV_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (rtvDescriptorSize * index);

		m_Device->CreateRenderTargetView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->m_Image,
			&ref_cast<ImageView>(descriptorImageInfos[0].imageView)->m_RTVDesc, descriptorWriteLocation);
	}
	//DSV
	else if (usage == Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		descriptorWriteLocation = m_DSV_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (dsvDescriptorSize * index);

		m_Device->CreateDepthStencilView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->m_Image,
			&ref_cast<ImageView>(descriptorImageInfos[0].imageView)->m_DSVDesc, descriptorWriteLocation);
	}
	//UAV
	else if(usage == Image::UsageBit::STORAGE_BIT)
	{
		descriptorWriteLocation = m_CBV_SRV_UAV_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (cbv_srv_uav_DescriptorSize * index);

		m_Device->CreateUnorderedAccessView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->m_Image, nullptr,
			&ref_cast<ImageView>(descriptorImageInfos[0].imageView)->m_UAVDesc, descriptorWriteLocation);
	}
	//SRV
	else
	{
		descriptorWriteLocation = m_CBV_SRV_UAV_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (cbv_srv_uav_DescriptorSize * index);

		m_Device->CreateShaderResourceView(ref_cast<Image>(ref_cast<ImageView>(descriptorImageInfos[0].imageView)->GetCreateInfo().pImage)->m_Image,
			/*&ref_cast<ImageView>(descriptorImageInfos[0].imageView)->m_SRVDesc*/0, descriptorWriteLocation);
	}
	
	//Sampler
	if (descriptorImageInfos[0].sampler)
	{
		descriptorWriteLocation = m_Sampler_DescHeapCPUHandle;
		descriptorWriteLocation.ptr += (samplerDescriptorSize * index);

		m_Device->CreateSampler(&ref_cast<Sampler>(descriptorImageInfos[0].sampler)->m_SamplerDesc, descriptorWriteLocation);
	}
}

void DescriptorSet::Update()
{
	m_RootParameters.clear();

	for (auto& descriptorSetLayout : m_CI.pDescriptorSetLayouts)
	{
		D3D12_ROOT_PARAMETER rootParameter = {};
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorTable;;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		m_RootParameters.push_back(rootParameter);
	}
}

