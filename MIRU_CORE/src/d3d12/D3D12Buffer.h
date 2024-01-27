#pragma once
#include "base/Buffer.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class Buffer final : public base::Buffer
	{
		//Methods
	public:
		Buffer(Buffer::CreateInfo* pCreateInfo);
		~Buffer();

	private:
		D3D12_RESOURCE_STATES ToD3D12BufferType(Buffer::UsageBit type) const;

		//Members
	public:
		ID3D12Device* m_Device;

		ID3D12Resource* m_Buffer;
		D3D12_RESOURCE_DESC m_ResourceDesc;
		D3D12_RESOURCE_STATES m_InitialResourceState;

		D3D12MA::Allocation* m_D3D12MAllocation;
		D3D12MA::ALLOCATION_DESC m_D3D12MAllocationDesc;
	};

	class BufferView final : public base::BufferView
	{
		//Methods
	public:
		BufferView(BufferView::CreateInfo* pCreateInfo);
		~BufferView();

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc;
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_UAVDesc;
		D3D12_INDEX_BUFFER_VIEW m_IBVDesc;
		D3D12_VERTEX_BUFFER_VIEW m_VBVDesc;

		D3D12_CPU_DESCRIPTOR_HANDLE m_CBVDescHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_UAVDescHandle = {};
	};
}
}