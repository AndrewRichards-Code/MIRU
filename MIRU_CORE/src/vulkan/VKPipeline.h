#pragma once
#if defined(MIRU_VULKAN)
#include "miru_core_common.h"
#include "crossplatform/Pipeline.h"

namespace miru
{
namespace vulkan
{
	class RenderPass final : public crossplatform::RenderPass
	{
		//Method
	public:
		RenderPass(RenderPass::CreateInfo* pCreateInfo);
		~RenderPass();

		//Members
	public:
		VkDevice& m_Device;

		VkRenderPass m_RenderPass;
		VkRenderPassCreateInfo m_RenderPassCI;

		std::vector<VkAttachmentDescription> m_AttachmentDescriptions;
		std::vector<std::array<std::vector<VkAttachmentReference>, 5>> m_AttachmentReferencesByUsageBySubpass;
		std::vector<VkSubpassDescription> m_SubpassDescriptions;
		std::vector<VkSubpassDependency> m_SubpassDependencies;
		#if defined(VK_KHR_multiview)
		VkRenderPassMultiviewCreateInfoKHR m_MultiviewCreateInfo;
		#endif
	};

	class Pipeline final : public crossplatform::Pipeline
	{
		//Methods
	public:
		Pipeline(Pipeline::CreateInfo* pCreateInfo);
		~Pipeline();

		std::vector<std::pair<crossplatform::ShaderGroupHandleType, std::vector<uint8_t>>> GetShaderGroupHandles() override;

		static VkFormat ToVkFormat(crossplatform::VertexType type);

		//Members
	public:
		VkDevice& m_Device;

		VkPipeline m_Pipeline;
		VkGraphicsPipelineCreateInfo m_GPCI = {};
		VkComputePipelineCreateInfo m_CPCI = {};
		VkRayTracingPipelineCreateInfoKHR m_RTPCI = {};

		VkPipelineLayout m_PipelineLayout;
		VkPipelineLayoutCreateInfo m_PLCI;

		std::vector<std::pair<crossplatform::ShaderGroupHandleType, std::vector<uint8_t>>> m_ShaderGroupHandles;
	};
}
}
#endif