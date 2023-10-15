#pragma once
#include "base/Framebuffer.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class Framebuffer final : public base::Framebuffer
	{
		//Methods
	public:
		Framebuffer(Framebuffer::CreateInfo* pCreateInfo);
		~Framebuffer();

		//Members
	public:
		VkDevice& m_Device;

		VkFramebuffer m_Framebuffer;
		VkFramebufferCreateInfo m_FramebufferCI;
	};
}
}