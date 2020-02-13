#pragma once
#include "common.h"
#include "crossplatform/Pipeline.h"

namespace miru
{
namespace d3d12
{
	class RenderPass final : public crossplatform::RenderPass
	{
		//Method
	public:
		RenderPass(RenderPass::CreateInfo* pCreateInfo);
		~RenderPass();

		//Members
	public:
		ID3D12Device* m_Device;

		std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderTargetDescriptions;
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStenciDescription;

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