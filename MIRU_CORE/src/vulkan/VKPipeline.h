#pragma once
#include "base/Pipeline.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class Pipeline final : public base::Pipeline
	{
		//Methods
	public:
		Pipeline(Pipeline::CreateInfo* pCreateInfo);
		~Pipeline();

		std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> GetShaderGroupHandles() override;

		static VkFormat ToVkFormat(base::VertexType type);

		//Members
	public:
		VkDevice m_Device;

		VkPipeline m_Pipeline;
		VkGraphicsPipelineCreateInfo m_GPCI = {};
		VkComputePipelineCreateInfo m_CPCI = {};
		VkRayTracingPipelineCreateInfoKHR m_RTPCI = {};

		VkPipelineLayout m_PipelineLayout;
		VkPipelineLayoutCreateInfo m_PLCI;

		std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> m_ShaderGroupHandles;
	};
}
}