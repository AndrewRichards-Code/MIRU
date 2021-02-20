#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKFramebuffer.h"
#include "VKPipeline.h"
#include "VKImage.h"

using namespace miru;
using namespace vulkan;

Framebuffer::Framebuffer(Framebuffer::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	std::vector<VkImageView> vkImageViewAttachements;
	vkImageViewAttachements.reserve(m_CI.attachments.size());
	for (auto& attachment : m_CI.attachments)
		vkImageViewAttachements.push_back(ref_cast<ImageView>(attachment)->m_ImageView);

	m_FramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_FramebufferCI.pNext = nullptr;
	m_FramebufferCI.flags = 0;
	m_FramebufferCI.renderPass = ref_cast<RenderPass>(m_CI.renderPass)->m_RenderPass;
	m_FramebufferCI.attachmentCount = static_cast<uint32_t>(vkImageViewAttachements.size());
	m_FramebufferCI.pAttachments = vkImageViewAttachements.data();
	m_FramebufferCI.width = m_CI.width;
	m_FramebufferCI.height = m_CI.height;
	m_FramebufferCI.layers = m_CI.layers;

	MIRU_ASSERT(vkCreateFramebuffer(m_Device, &m_FramebufferCI, nullptr, &m_Framebuffer), "ERROR: VULKAN: Failed to create Framebuffer.");
	VKSetName<VkFramebuffer>(m_Device, (uint64_t)m_Framebuffer, m_CI.debugName);
}

Framebuffer::~Framebuffer()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
}
#endif