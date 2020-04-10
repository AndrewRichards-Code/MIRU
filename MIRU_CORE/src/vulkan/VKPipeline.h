#pragma once
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
	};

	class Pipeline final : public crossplatform::Pipeline
	{
		//Methods
	public:
		Pipeline(Pipeline::CreateInfo* pCreateInfo);
		~Pipeline();

	private:
		VkFormat ToVkFormat(crossplatform::VertexType type);

		//Members
	public:
		VkDevice& m_Device;

		VkPipeline m_Pipeline;
		VkGraphicsPipelineCreateInfo m_GPCI = {};
		VkComputePipelineCreateInfo m_CPCI = {};

		VkPipelineLayout m_PipelineLayout;
		VkPipelineLayoutCreateInfo m_PLCI;
	};
}
}