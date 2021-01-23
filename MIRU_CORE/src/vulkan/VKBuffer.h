#pragma once
#if defined(MIRU_VULKAN)
#include "crossplatform/Buffer.h"

namespace miru
{
namespace vulkan
{
	class Buffer final : public crossplatform::Buffer
	{
		//Methods
	public:
		Buffer(Buffer::CreateInfo* pCreateInfo);
		~Buffer();

	private:
		VkBufferUsageFlags ToVKBufferType(Buffer::UsageBit type);

		//Members
	public:
		VkDevice& m_Device;

		VkBuffer m_Buffer;
		VkBufferCreateInfo m_BufferCI = {};
		
		VmaAllocation m_VmaAllocation;
		VmaAllocationCreateInfo m_VmaACI;
		VmaAllocationInfo m_VmaAI;
	};

	class BufferView final : public crossplatform::BufferView
	{
	public:
		//Methods
		BufferView(CreateInfo* pCreateInfo);
		~BufferView();

		//Members
	public:
		VkDevice& m_Device;

		VkBufferView m_BufferView;
		VkBufferViewCreateInfo m_BufferViewCI = {};
	};
}
}
#endif