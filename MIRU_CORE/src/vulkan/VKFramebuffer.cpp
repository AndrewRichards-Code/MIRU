#include "common.h"
#include "VKFramebuffer.h"
#include "VKPipeline.h"
#include "VKImage.h"

using namespace miru;
using namespace vulkan;

Framebuffer::Framebuffer(Framebuffer::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	std::vector<VkImageView> vkImageViewAttachements;
	vkImageViewAttachements.reserve(m_CI.attachments.size());
	for (auto& attachment : m_CI.attachments)
		vkImageViewAttachements.push_back(std::dynamic_pointer_cast<ImageView>(attachment)->m_ImageView);

	m_FramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_FramebufferCI.pNext = nullptr;
	m_FramebufferCI.flags = 0;
	m_FramebufferCI.renderPass = std::dynamic_pointer_cast<RenderPass>(m_CI.renderPass)->m_RenderPass;
	m_FramebufferCI.attachmentCount = static_cast<uint32_t>(vkImageViewAttachements.size());
	m_FramebufferCI.pAttachments = vkImageViewAttachements.data();
	m_FramebufferCI.width = m_CI.width;
	m_FramebufferCI.height = m_CI.height;
	m_FramebufferCI.layers = m_CI.layers;

	MIRU_ASSERT(vkCreateFramebuffer(m_Device, &m_FramebufferCI, nullptr, &m_Framebuffer), "ERROR: VULKAN: Failed to create Framebuffer.");
	VKSetName<VkBuffer>(m_Device, (uint64_t)m_Framebuffer, m_CI.debugName);
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
}