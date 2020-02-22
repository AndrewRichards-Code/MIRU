#pragma once
#include "crossplatform/Buffer.h"

namespace miru
{
namespace d3d12
{
	class Buffer final : public crossplatform::Buffer
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
		D3D12_RESOURCE_STATES m_CurrentResourceState;
		D3D12_RESOURCE_ALLOCATION_INFO m_AllocationInfo;
	};

	class BufferView final : public crossplatform::BufferView
	{
		//Methods
	public:
		BufferView(BufferView::CreateInfo* pCreateInfo);
		~BufferView();

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc;
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_UAVDesc;
		D3D12_INDEX_BUFFER_VIEW m_IBVDesc;
		D3D12_VERTEX_BUFFER_VIEW m_VBVDesc;

		D3D12_CPU_DESCRIPTOR_HANDLE m_CBVDescHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_UAVDescHandle = {};
	};
}
}