#pragma once
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
		VkMemoryRequirements m_MemoryRequirements;
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