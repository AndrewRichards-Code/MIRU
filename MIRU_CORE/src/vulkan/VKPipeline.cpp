#include "common.h"
#include "VKPipeline.h"
#include "VKDescriptorPoolSet.h"
#include "VKShader.h"

using namespace miru;
using namespace vulkan;

//RenderPass
RenderPass::RenderPass(RenderPass::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_AttachmentDescriptions.reserve(m_CI.attachments.size());
	for (auto& attachment : m_CI.attachments)
		m_AttachmentDescriptions.push_back({
		static_cast<VkAttachmentDescriptionFlags>(0),
		static_cast<VkFormat>(attachment.format),
		static_cast<VkSampleCountFlagBits>(attachment.samples),
		static_cast<VkAttachmentLoadOp>(attachment.loadOp),
		static_cast<VkAttachmentStoreOp>(attachment.storeOp),
		static_cast<VkAttachmentLoadOp>(attachment.stencilLoadOp),
		static_cast<VkAttachmentStoreOp>(attachment.stencilStoreOp),
		static_cast<VkImageLayout>(attachment.initialLayout),
		static_cast<VkImageLayout>(attachment.finalLayout),
			});

	for (auto& subpassDescription : m_CI.subpassDescriptions)
	{
		m_AttachmentReferencesByUsageBySubpass.push_back({});

		for (auto& input : subpassDescription.inputAttachments)
			m_AttachmentReferencesByUsageBySubpass.back()[0].push_back({ input.attachmentIndex, static_cast<VkImageLayout>(input.layout) });

		for (auto& colour : subpassDescription.colourAttachments)
			m_AttachmentReferencesByUsageBySubpass.back()[1].push_back({ colour.attachmentIndex, static_cast<VkImageLayout>(colour.layout) });

		for (auto& resolve : subpassDescription.resolveAttachments)
			m_AttachmentReferencesByUsageBySubpass.back()[2].push_back({ resolve.attachmentIndex, static_cast<VkImageLayout>(resolve.layout) });

		for (auto& depth : subpassDescription.depthStencilAttachment)
			m_AttachmentReferencesByUsageBySubpass.back()[3].push_back({depth.attachmentIndex, static_cast<VkImageLayout>(depth.layout) });

		for (auto& preserve : subpassDescription.preseverseAttachments)
			m_AttachmentReferencesByUsageBySubpass.back()[4].push_back({ preserve.attachmentIndex, static_cast<VkImageLayout>(preserve.layout) });
	}

	m_SubpassDescriptions.reserve(m_CI.subpassDescriptions.size());
	size_t i = 0;
	for (auto& subpassDescription : m_CI.subpassDescriptions)
	{
		m_SubpassDescriptions.push_back({
			static_cast<VkSubpassDescriptionFlags>(0),
			static_cast<VkPipelineBindPoint>(subpassDescription.pipelineType),
			static_cast<uint32_t>(m_AttachmentReferencesByUsageBySubpass[i][0].size()),
			m_AttachmentReferencesByUsageBySubpass[i][0].data(),
			static_cast<uint32_t>(m_AttachmentReferencesByUsageBySubpass[i][1].size()),
			m_AttachmentReferencesByUsageBySubpass[i][1].data(),
			m_AttachmentReferencesByUsageBySubpass[i][2].data(),
			m_AttachmentReferencesByUsageBySubpass[i][3].data(),
			static_cast<uint32_t>(m_AttachmentReferencesByUsageBySubpass[i][4].size()),
			reinterpret_cast<uint32_t*>(m_AttachmentReferencesByUsageBySubpass[i][4].data()),
			});
		i++;
	}

	m_SubpassDependencies.reserve(m_CI.subpassDependencies.size());
	for (auto& dependency : m_CI.subpassDependencies)
		m_SubpassDependencies.push_back({
			dependency.srcSubpass,
			dependency.dstSubpass,
			static_cast<VkPipelineStageFlags>(dependency.srcStage),
			static_cast<VkPipelineStageFlags>(dependency.dstStage),
			static_cast<VkAccessFlags>(dependency.srcAccess),
			static_cast<VkAccessFlags>(dependency.dstAccess),
			static_cast<VkDependencyFlags>(0)
			});
	
	m_RenderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	m_RenderPassCI.pNext = nullptr;
	m_RenderPassCI.flags = 0;
	m_RenderPassCI.attachmentCount = static_cast<uint32_t>(m_AttachmentDescriptions.size());
	m_RenderPassCI.pAttachments = m_AttachmentDescriptions.data();
	m_RenderPassCI.subpassCount = static_cast<uint32_t>(m_SubpassDescriptions.size());
	m_RenderPassCI.pSubpasses = m_SubpassDescriptions.data();
	m_RenderPassCI.dependencyCount = static_cast<uint32_t>(m_SubpassDependencies.size());
	m_RenderPassCI.pDependencies = m_SubpassDependencies.data();

	MIRU_ASSERT(vkCreateRenderPass(m_Device, &m_RenderPassCI, nullptr, &m_RenderPass), "ERROR: VULKAN: Failed to create RenderPass.");
	VKSetName<VkRenderPass>(m_Device, (uint64_t)m_RenderPass, m_CI.debugName);
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
}

//Pipeline
Pipeline::Pipeline(Pipeline::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
	vkDescriptorSetLayouts.reserve(m_CI.layout.descriptorSetLayouts.size());
	for(auto& descriptorSetLayout : m_CI.layout.descriptorSetLayouts)
		vkDescriptorSetLayouts.push_back(ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorSetLayout);

	std::vector<VkPushConstantRange> vkPushConstantRanges;
	vkPushConstantRanges.reserve(m_CI.layout.pushConstantRanges.size());
	for (auto& pushConstantRange : m_CI.layout.pushConstantRanges)
		vkPushConstantRanges.push_back({static_cast<VkShaderStageFlags>(pushConstantRange.stages), pushConstantRange.offset, pushConstantRange.size});

	m_PLCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	m_PLCI.pNext = nullptr;
	m_PLCI.flags = 0;
	m_PLCI.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	m_PLCI.pSetLayouts = vkDescriptorSetLayouts.data();
	m_PLCI.pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size());
	m_PLCI.pPushConstantRanges = vkPushConstantRanges.data();
	
	MIRU_ASSERT(vkCreatePipelineLayout(m_Device, &m_PLCI, nullptr, &m_PipelineLayout), "ERROR: VULKAN: Failed to create PipelineLayout.");
	VKSetName<VkPipelineLayout>(m_Device, (uint64_t)m_PipelineLayout, (std::string(m_CI.debugName) + std::string(" : PipelineLayout")).c_str());

	if (m_CI.type == crossplatform::PipelineType::GRAPHICS)
	{
		//ShaderStages
		std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
		vkShaderStages.reserve(m_CI.shaders.size());
		for (auto& shader : m_CI.shaders)
			vkShaderStages.push_back(ref_cast<Shader>(shader)->m_ShaderStageCI);

		//VertexInput
		std::vector<VkVertexInputBindingDescription> vkVertexInputBindingDescriptions;
		vkVertexInputBindingDescriptions.reserve(m_CI.vertexInputState.vertexInputBindingDescriptions.size());
		for (auto& vertexInputBindingDescription : m_CI.vertexInputState.vertexInputBindingDescriptions)
			vkVertexInputBindingDescriptions.push_back({ vertexInputBindingDescription.binding, vertexInputBindingDescription.stride, 
				static_cast<VkVertexInputRate>(vertexInputBindingDescription.inputRate) });

		std::vector<VkVertexInputAttributeDescription> vkVertexInputAttributeDescriptions;
		vkVertexInputAttributeDescriptions.reserve(m_CI.vertexInputState.vertexInputAttributeDescriptions.size());
		for (auto& vertexInputAttributeDescription : m_CI.vertexInputState.vertexInputAttributeDescriptions)
			vkVertexInputAttributeDescriptions.push_back({ vertexInputAttributeDescription.location, vertexInputAttributeDescription.binding, 
				ToVkFormat(vertexInputAttributeDescription.vertexType), vertexInputAttributeDescription.offset });

		VkPipelineVertexInputStateCreateInfo vkVertexInputState;
		vkVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vkVertexInputState.pNext = nullptr;
		vkVertexInputState.flags = 0;
		vkVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vkVertexInputBindingDescriptions.size());
		vkVertexInputState.pVertexBindingDescriptions = vkVertexInputBindingDescriptions.data();
		vkVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vkVertexInputAttributeDescriptions.size());
		vkVertexInputState.pVertexAttributeDescriptions = vkVertexInputAttributeDescriptions.data();

		//InputAssembly
		VkPipelineInputAssemblyStateCreateInfo vkInputAssemblyState;
		vkInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vkInputAssemblyState.pNext = nullptr;
		vkInputAssemblyState.flags = 0;
		vkInputAssemblyState.topology = static_cast<VkPrimitiveTopology>(m_CI.inputAssemblyState.topology);
		vkInputAssemblyState.primitiveRestartEnable = m_CI.inputAssemblyState.primitiveRestartEnable;

		//Tessellation
		VkPipelineTessellationStateCreateInfo vkTessellationState;
		vkTessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		vkTessellationState.pNext = nullptr;
		vkTessellationState.flags = 0;
		vkTessellationState.patchControlPoints = m_CI.tessellationState.patchControlPoints;

		//Viewport
		std::vector<VkViewport> vkViewports;
		vkViewports.reserve(m_CI.viewportState.viewports.size());
		for (auto& viewport : m_CI.viewportState.viewports)
			vkViewports.push_back({ viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth });

		std::vector<VkRect2D> vkRect2D;
		vkRect2D.reserve(m_CI.viewportState.scissors.size());
		for (auto& scissor : m_CI.viewportState.scissors)
			vkRect2D.push_back({{scissor.offset.x, scissor.offset.y}, {scissor.extent.width, scissor.extent.height}});

		VkPipelineViewportStateCreateInfo vkViewportState;
		vkViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vkViewportState.pNext = nullptr;
		vkViewportState.flags = 0;
		vkViewportState.viewportCount = static_cast<uint32_t>(vkViewports.size());
		vkViewportState.pViewports = vkViewports.data();
		vkViewportState.scissorCount = static_cast<uint32_t>(vkRect2D.size());
		vkViewportState.pScissors = vkRect2D.data();

		//Rasterisation
		VkPipelineRasterizationStateCreateInfo vkRasterisationState;
		vkRasterisationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		vkRasterisationState.pNext = nullptr;
		vkRasterisationState.flags = 0;
		vkRasterisationState.depthClampEnable = m_CI.rasterisationState.depthClampEnable;
		vkRasterisationState.rasterizerDiscardEnable = m_CI.rasterisationState.rasteriserDiscardEnable;
		vkRasterisationState.polygonMode = static_cast<VkPolygonMode>(m_CI.rasterisationState.polygonMode);
		vkRasterisationState.cullMode = static_cast<VkCullModeFlagBits>(m_CI.rasterisationState.cullMode);
		vkRasterisationState.frontFace = static_cast<VkFrontFace>(m_CI.rasterisationState.frontFace);
		vkRasterisationState.depthBiasEnable = m_CI.rasterisationState.depthBiasEnable;
		vkRasterisationState.depthBiasConstantFactor = m_CI.rasterisationState.depthBiasConstantFactor;
		vkRasterisationState.depthBiasClamp = m_CI.rasterisationState.depthBiasClamp;
		vkRasterisationState.depthBiasSlopeFactor = m_CI.rasterisationState.depthBiasSlopeFactor;
		vkRasterisationState.lineWidth = m_CI.rasterisationState.lineWidth;

		//Multisample
		VkPipelineMultisampleStateCreateInfo vkMultisampleState;
		vkMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		vkMultisampleState.pNext = nullptr;
		vkMultisampleState.flags = 0;
		vkMultisampleState.rasterizationSamples = static_cast<VkSampleCountFlagBits>(m_CI.multisampleState.rasterisationSamples);
		vkMultisampleState.sampleShadingEnable = m_CI.multisampleState.sampleShadingEnable;
		vkMultisampleState.minSampleShading = m_CI.multisampleState.minSampleShading;
		vkMultisampleState.pSampleMask = nullptr;
		vkMultisampleState.alphaToCoverageEnable = m_CI.multisampleState.alphaToCoverageEnable;
		vkMultisampleState.alphaToOneEnable = m_CI.multisampleState.alphaToOneEnable;

		//DepthStencil
		VkPipelineDepthStencilStateCreateInfo vkDepthStencilState;
		vkDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		vkDepthStencilState.pNext = nullptr;
		vkDepthStencilState.flags = 0;
		vkDepthStencilState.depthTestEnable = m_CI.depthStencilState.depthTestEnable;
		vkDepthStencilState.depthWriteEnable = m_CI.depthStencilState.depthWriteEnable;
		vkDepthStencilState.depthCompareOp = static_cast<VkCompareOp>(m_CI.depthStencilState.depthCompareOp);
		vkDepthStencilState.depthBoundsTestEnable = m_CI.depthStencilState.depthBoundsTestEnable;
		vkDepthStencilState.stencilTestEnable = m_CI.depthStencilState.stencilTestEnable;
		vkDepthStencilState.front.failOp = static_cast<VkStencilOp>(m_CI.depthStencilState.front.failOp);
		vkDepthStencilState.front.passOp = static_cast<VkStencilOp>(m_CI.depthStencilState.front.passOp);
		vkDepthStencilState.front.depthFailOp = static_cast<VkStencilOp>(m_CI.depthStencilState.front.depthFailOp);
		vkDepthStencilState.front.compareOp = static_cast<VkCompareOp>(m_CI.depthStencilState.front.compareOp);
		vkDepthStencilState.front.compareMask = m_CI.depthStencilState.front.compareMask;
		vkDepthStencilState.front.writeMask = m_CI.depthStencilState.front.writeMask;
		vkDepthStencilState.front.reference = m_CI.depthStencilState.front.reference;
		vkDepthStencilState.back.failOp = static_cast<VkStencilOp>(m_CI.depthStencilState.back.failOp);
		vkDepthStencilState.back.passOp = static_cast<VkStencilOp>(m_CI.depthStencilState.back.passOp);
		vkDepthStencilState.back.depthFailOp = static_cast<VkStencilOp>(m_CI.depthStencilState.back.depthFailOp);
		vkDepthStencilState.back.compareOp = static_cast<VkCompareOp>(m_CI.depthStencilState.back.compareOp);
		vkDepthStencilState.back.compareMask = m_CI.depthStencilState.back.compareMask;
		vkDepthStencilState.back.writeMask = m_CI.depthStencilState.back.writeMask;
		vkDepthStencilState.back.reference = m_CI.depthStencilState.back.reference;
		vkDepthStencilState.minDepthBounds = m_CI.depthStencilState.minDepthBounds;
		vkDepthStencilState.maxDepthBounds = m_CI.depthStencilState.maxDepthBounds;

		//ColourBlend
		std::vector<VkPipelineColorBlendAttachmentState> vkPipelineColorBlendAttachmentStates;
		vkPipelineColorBlendAttachmentStates.reserve(m_CI.colourBlendState.attachments.size());
		for (auto& attachment : m_CI.colourBlendState.attachments)
			vkPipelineColorBlendAttachmentStates.push_back(
				{ 
					attachment.blendEnable,
					static_cast<VkBlendFactor>(attachment.srcColourBlendFactor),
					static_cast<VkBlendFactor>(attachment.dstColourBlendFactor),
					static_cast<VkBlendOp>(attachment.colorBlendOp),
					static_cast<VkBlendFactor>(attachment.srcAlphaBlendFactor),
					static_cast<VkBlendFactor>(attachment.dstAlphaBlendFactor),
					static_cast<VkBlendOp>(attachment.alphaBlendOp),
					static_cast<VkColorComponentFlags>(attachment.colourWriteMask)
				});


		VkPipelineColorBlendStateCreateInfo vkColourBlendState;
		vkColourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		vkColourBlendState.pNext = nullptr;
		vkColourBlendState.flags = 0;
		vkColourBlendState.logicOpEnable = m_CI.colourBlendState.logicOpEnable;
		vkColourBlendState.logicOp = static_cast<VkLogicOp>(m_CI.colourBlendState.logicOp);
		vkColourBlendState.attachmentCount = static_cast<uint32_t>(vkPipelineColorBlendAttachmentStates.size());
		vkColourBlendState.pAttachments = vkPipelineColorBlendAttachmentStates.data();
		vkColourBlendState.blendConstants[0] = m_CI.colourBlendState.blendConstants[0];
		vkColourBlendState.blendConstants[1] = m_CI.colourBlendState.blendConstants[1];
		vkColourBlendState.blendConstants[2] = m_CI.colourBlendState.blendConstants[2];
		vkColourBlendState.blendConstants[3] = m_CI.colourBlendState.blendConstants[3];

		//Dynamic
		std::vector<VkDynamicState> vkDynamicStates;
		vkDynamicStates.reserve(m_CI.dynamicStates.dynamicStates.size());
		for (auto& dynamicState : m_CI.dynamicStates.dynamicStates)
			vkDynamicStates.push_back(static_cast<VkDynamicState>(dynamicState));

		VkPipelineDynamicStateCreateInfo vkDynamicState;
		vkDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		vkDynamicState.pNext = nullptr;
		vkDynamicState.flags = 0;
		vkDynamicState.dynamicStateCount = static_cast<uint32_t>(vkDynamicStates.size());
		vkDynamicState.pDynamicStates = vkDynamicStates.data();


		//Fill Vulkan structure
		m_GPCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		m_GPCI.pNext = nullptr;
		m_GPCI.flags = 0;
		m_GPCI.stageCount = static_cast<uint32_t>(vkShaderStages.size());
		m_GPCI.pStages = vkShaderStages.data();
		m_GPCI.pVertexInputState = &vkVertexInputState;
		m_GPCI.pInputAssemblyState = &vkInputAssemblyState;
		m_GPCI.pTessellationState = &vkTessellationState;
		m_GPCI.pViewportState = &vkViewportState;
		m_GPCI.pRasterizationState = &vkRasterisationState;
		m_GPCI.pMultisampleState = &vkMultisampleState;
		m_GPCI.pDepthStencilState = &vkDepthStencilState;
		m_GPCI.pColorBlendState = &vkColourBlendState;
		m_GPCI.pDynamicState = &vkDynamicState;
		m_GPCI.layout = m_PipelineLayout;
		m_GPCI.renderPass = ref_cast<RenderPass>(m_CI.renderPass)->m_RenderPass;
		m_GPCI.subpass = m_CI.subpassIndex;
		m_GPCI.basePipelineHandle = VK_NULL_HANDLE;
		m_GPCI.basePipelineIndex = -1;

		MIRU_ASSERT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &m_GPCI, nullptr, &m_Pipeline), "ERROR: VULKAN: Failed to create Graphics Pipeline.");
		VKSetName<VkPipelineLayout>(m_Device, (uint64_t)m_Pipeline, (std::string(m_CI.debugName) + std::string(" : Graphics Pipeline")).c_str());
	}
	else if (m_CI.type == crossplatform::PipelineType::COMPUTE)
	{
		m_CPCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		m_CPCI.pNext = nullptr;
		m_CPCI.flags = 0;
		m_CPCI.stage = ref_cast<Shader>(m_CI.shaders[0])->m_ShaderStageCI;
		m_CPCI.layout = m_PipelineLayout;
		m_CPCI.basePipelineHandle = VK_NULL_HANDLE;
		m_CPCI.basePipelineIndex = -1;
		
		MIRU_ASSERT(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &m_CPCI, nullptr, &m_Pipeline), "ERROR: VULKAN: Failed to create Compute Pipeline.");
		VKSetName<VkPipelineLayout>(m_Device, (uint64_t)m_Pipeline, (std::string(m_CI.debugName) + std::string(" : Compute Pipeline")).c_str());
	}
	else
		MIRU_ASSERT(true, "ERROR: VULKAN: Unknown pipeline type.");
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
}

VkFormat Pipeline::ToVkFormat(crossplatform::VertexType type)
{
	switch (type)
	{
	case miru::crossplatform::VertexType::FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case miru::crossplatform::VertexType::VEC2:
		return VK_FORMAT_R32G32_SFLOAT;
	case miru::crossplatform::VertexType::VEC3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case miru::crossplatform::VertexType::VEC4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case miru::crossplatform::VertexType::INT:
		return VK_FORMAT_R32_SINT;
	case miru::crossplatform::VertexType::IVEC2:
		return VK_FORMAT_R32G32_SINT;
	case miru::crossplatform::VertexType::IVEC3:
		return VK_FORMAT_R32G32B32_SINT;
	case miru::crossplatform::VertexType::IVEC4:
		return VK_FORMAT_R32G32B32A32_SINT;
	case miru::crossplatform::VertexType::UINT:
		return VK_FORMAT_R32_UINT;
	case miru::crossplatform::VertexType::UVEC2:
		return VK_FORMAT_R32G32_UINT;
	case miru::crossplatform::VertexType::UVEC3:
		return VK_FORMAT_R32G32B32_UINT;
	case miru::crossplatform::VertexType::UVEC4:
		return VK_FORMAT_R32G32B32A32_UINT;
	case miru::crossplatform::VertexType::DOUBLE:
		return VK_FORMAT_R64_SFLOAT;
	case miru::crossplatform::VertexType::DVEC2:
		return VK_FORMAT_R64G64_SFLOAT;
	case miru::crossplatform::VertexType::DVEC3:
		return VK_FORMAT_R64G64B64_SFLOAT;
	case miru::crossplatform::VertexType::DVEC4:
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}
