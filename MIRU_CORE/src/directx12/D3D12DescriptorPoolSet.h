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

		//[0] == HEAP_TYPE_SAMPLER and [1] == HEAP_TYPE_CBV_SRV_UAV
		//[2] == HEAP_TYPE_RTV and [3] == HEAP_TYPE_DSV
		ID3D12DescriptorHeap* m_DescriptorPool[4] = {};
		D3D12_DESCRIPTOR_HEAP_DESC m_DescriptorPoolDesc[4] = {};
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
		D3D12_ROOT_DESCRIPTOR_TABLE m_DescriptorTable;
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

		//Members
	public:
		ID3D12Device* m_Device;
		std::vector<D3D12_ROOT_PARAMETER> m_RootParameters;

		D3D12_CPU_DESCRIPTOR_HANDLE m_CBV_SRV_UAV_DescHeapCPUHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_Sampler_DescHeapCPUHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_RTV_DescHeapCPUHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_DSV_DescHeapCPUHandle = {};
	};
}
}