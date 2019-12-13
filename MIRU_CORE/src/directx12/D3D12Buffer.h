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
	};
}
}