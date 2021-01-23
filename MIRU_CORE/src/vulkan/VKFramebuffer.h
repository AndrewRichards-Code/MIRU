#pragma once
#if defined(MIRU_VULKAN)
#include "crossplatform/Framebuffer.h"

namespace miru
{
namespace vulkan
{
	class Framebuffer final : public crossplatform::Framebuffer
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
#endif