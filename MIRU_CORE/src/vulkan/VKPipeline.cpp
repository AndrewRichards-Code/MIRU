#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKPipeline.h"
#include "VKDescriptorPoolSet.h"
#include "VKShader.h"
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

//RenderPass
RenderPass::RenderPass(RenderPass::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

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
			static_cast<VkDependencyFlags>(dependency.dependencies)
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

	if (!m_CI.multiview.viewMasks.empty())
	{
		m_MultiviewCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
		m_MultiviewCreateInfo.pNext = nullptr;
		m_MultiviewCreateInfo.subpassCount = static_cast<uint32_t>(m_CI.multiview.viewMasks.size());
		m_MultiviewCreateInfo.pViewMasks = m_CI.multiview.viewMasks.size() ? m_CI.multiview.viewMasks.data() : nullptr;
		m_MultiviewCreateInfo.dependencyCount = static_cast<uint32_t>(m_CI.multiview.viewOffsets.size());
		m_MultiviewCreateInfo.pViewOffsets = m_CI.multiview.viewOffsets.size() ? m_CI.multiview.viewOffsets.data() : nullptr;
		m_MultiviewCreateInfo.correlationMaskCount = static_cast<uint32_t>(m_CI.multiview.correlationMasks.size());
		m_MultiviewCreateInfo.pCorrelationMasks = m_CI.multiview.correlationMasks.size() ? m_CI.multiview.correlationMasks.data() : nullptr;
		m_RenderPassCI.pNext = &m_MultiviewCreateInfo;
	}

	MIRU_ASSERT(vkCreateRenderPass(m_Device, &m_RenderPassCI, nullptr, &m_RenderPass), "ERROR: VULKAN: Failed to create RenderPass.");
	VKSetName<VkRenderPass>(m_Device, m_RenderPass, m_CI.debugName);
}

RenderPass::~RenderPass()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
}

//Pipeline
Pipeline::Pipeline(Pipeline::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

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
	VKSetName<VkPipelineLayout>(m_Device, m_PipelineLayout, m_CI.debugName + " : PipelineLayout");

	if (m_CI.type == base::PipelineType::GRAPHICS)
	{
		//ShaderStages
		std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
		vkShaderStages.reserve(m_CI.shaders.size());
		for (auto& shader : m_CI.shaders)
			vkShaderStages.push_back(ref_cast<Shader>(shader)->m_ShaderStageCIs[0]);

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
		vkMultisampleState.pSampleMask = static_cast<VkSampleMask*>(&m_CI.multisampleState.sampleMask);
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
					static_cast<VkBlendOp>(attachment.colourBlendOp),
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

		//Dynamic Rendering
		VkPipelineRenderingCreateInfo vkPipelineRenderingCI;
		vkPipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		vkPipelineRenderingCI.pNext = nullptr;
		vkPipelineRenderingCI.viewMask = m_CI.dynamicRendering.viewMask;
		vkPipelineRenderingCI.colorAttachmentCount = static_cast<uint32_t>(m_CI.dynamicRendering.colourAttachmentFormats.size());
		vkPipelineRenderingCI.pColorAttachmentFormats = reinterpret_cast<VkFormat*>(m_CI.dynamicRendering.colourAttachmentFormats.data());
		vkPipelineRenderingCI.depthAttachmentFormat = static_cast<VkFormat>(m_CI.dynamicRendering.depthAttachmentFormat);
		vkPipelineRenderingCI.stencilAttachmentFormat = static_cast<VkFormat>(m_CI.dynamicRendering.stencilAttachmentFormat);

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
		if (m_CI.renderPass)
			m_GPCI.renderPass = ref_cast<RenderPass>(m_CI.renderPass)->m_RenderPass;
		else
			m_GPCI.pNext = &vkPipelineRenderingCI;
		m_GPCI.subpass = m_CI.subpassIndex;
		m_GPCI.basePipelineHandle = VK_NULL_HANDLE;
		m_GPCI.basePipelineIndex = -1;

		MIRU_ASSERT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &m_GPCI, nullptr, &m_Pipeline), "ERROR: VULKAN: Failed to create Graphics Pipeline.");
		VKSetName<VkPipeline>(m_Device, m_Pipeline, m_CI.debugName + " : Graphics Pipeline");
	}
	else if (m_CI.type == base::PipelineType::COMPUTE)
	{
		m_CPCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		m_CPCI.pNext = nullptr;
		m_CPCI.flags = 0;
		m_CPCI.stage = ref_cast<Shader>(m_CI.shaders[0])->m_ShaderStageCIs[0];
		m_CPCI.layout = m_PipelineLayout;
		m_CPCI.basePipelineHandle = VK_NULL_HANDLE;
		m_CPCI.basePipelineIndex = -1;
		
		MIRU_ASSERT(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &m_CPCI, nullptr, &m_Pipeline), "ERROR: VULKAN: Failed to create Compute Pipeline.");
		VKSetName<VkPipeline>(m_Device, m_Pipeline, m_CI.debugName + " : Compute Pipeline");
	}
	else if (m_CI.type == base::PipelineType::RAY_TRACING)
	{
		//ShaderStages
		std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
		for (auto& shader : m_CI.shaders)
		{
			for (const auto& vkShaderStage : ref_cast<Shader>(shader)->m_ShaderStageCIs)
			{
				vkShaderStages.push_back(vkShaderStage);
			}
		}

		//LibraryInterface
		VkRayTracingPipelineInterfaceCreateInfoKHR vkInterfaceInfo;
		vkInterfaceInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR;
		vkInterfaceInfo.pNext = nullptr;
		vkInterfaceInfo.maxPipelineRayPayloadSize = m_CI.rayTracingInfo.maxPayloadSize;
		vkInterfaceInfo.maxPipelineRayHitAttributeSize = m_CI.rayTracingInfo.maxHitAttributeSize;

		//ShaderGroupInfo
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> vkShaderGroupInfos;
		vkShaderGroupInfos.reserve(m_CI.shaderGroupInfos.size());
		for (auto& shaderGroupInfo : m_CI.shaderGroupInfos)
		{
			VkRayTracingShaderGroupCreateInfoKHR vkShaderGroupInfo;
			vkShaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			vkShaderGroupInfo.pNext = nullptr;
			vkShaderGroupInfo.type = static_cast<VkRayTracingShaderGroupTypeKHR>(shaderGroupInfo.type);
			vkShaderGroupInfo.generalShader = shaderGroupInfo.generalShader;
			vkShaderGroupInfo.closestHitShader = shaderGroupInfo.closestHitShader;
			vkShaderGroupInfo.anyHitShader = shaderGroupInfo.anyHitShader;
			vkShaderGroupInfo.intersectionShader = shaderGroupInfo.intersectionShader;
			vkShaderGroupInfo.pShaderGroupCaptureReplayHandle = nullptr;
			vkShaderGroupInfos.push_back(vkShaderGroupInfo);
		}

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
		m_RTPCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		m_RTPCI.pNext = nullptr;
		m_RTPCI.flags = 0;
		m_RTPCI.stageCount = static_cast<uint32_t>(vkShaderStages.size());
		m_RTPCI.pStages = vkShaderStages.data();
		m_RTPCI.groupCount = static_cast<uint32_t>(vkShaderGroupInfos.size());
		m_RTPCI.pGroups = vkShaderGroupInfos.data();
		m_RTPCI.maxPipelineRayRecursionDepth = m_CI.rayTracingInfo.maxRecursionDepth;
		m_RTPCI.pLibraryInfo = nullptr;
		m_RTPCI.pLibraryInterface;
		m_RTPCI.pDynamicState = &vkDynamicState;
		m_RTPCI.layout = m_PipelineLayout;
		m_RTPCI.basePipelineHandle = VK_NULL_HANDLE;
		m_RTPCI.basePipelineIndex = -1;
		MIRU_ASSERT(vkCreateRayTracingPipelinesKHR(m_Device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &m_RTPCI, nullptr, &m_Pipeline), "ERROR: VULKAN: Failed to create Ray Tracing Pipeline.");
		VKSetName<VkPipeline>(m_Device, m_Pipeline, m_CI.debugName + " : Ray Tracing Pipeline");

		//Get ShaderHandles
		const ContextRef& vkContext = ref_cast<Context>(m_CI.rayTracingInfo.allocator->GetCreateInfo().context);
		uint32_t vkHandleSize = vkContext->m_PhysicalDevices.m_PDIs[0].m_RayTracingPipelineProperties.shaderGroupHandleSize;
		uint32_t vkHandleSizeAligned = vkContext->m_PhysicalDevices.m_PDIs[0].m_RayTracingPipelineProperties.shaderGroupHandleAlignment;

		const uint32_t& handleSize = vkHandleSize;
		const size_t handleSizeAligned = arc::Align(vkHandleSize, vkHandleSizeAligned);
		const size_t shaderGroupHandleDataSize = static_cast<size_t>(m_RTPCI.groupCount * handleSizeAligned);

		//Get ShaderGroupHandle - Handles should return in the order specific in VkRayTracingPipelineCreateInfoKHR::pStages.
		std::vector<uint8_t> shaderGroupHandles(shaderGroupHandleDataSize);
		MIRU_ASSERT(vkGetRayTracingShaderGroupHandlesKHR(m_Device, m_Pipeline, 0, m_RTPCI.groupCount, shaderGroupHandleDataSize, shaderGroupHandles.data()), "ERROR: VULKAN: Failed to get Ray Tracing Pipeline Shader Group Handles.");

		//We need to bundles the handles together by type.
		//Get the indices per type.
		size_t raygenCount = 0, missCount = 0, hitCount = 0, callableCount = 0;
		std::map<base::ShaderGroupHandleType, std::vector<size_t>> shaderGroupIndicesPerType;
		size_t idx = 0;
		for (auto& shaderGroupInfo : vkShaderGroupInfos)
		{
			if (shaderGroupInfo.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR)
			{
				const VkShaderStageFlagBits& vkStage = vkShaderStages[shaderGroupInfo.generalShader].stage;
				if (vkStage == VK_SHADER_STAGE_RAYGEN_BIT_KHR && raygenCount == 0)
				{
					shaderGroupIndicesPerType[base::ShaderGroupHandleType::RAYGEN].push_back(idx);
					raygenCount++;
				}
				else if (vkStage == VK_SHADER_STAGE_MISS_BIT_KHR)
				{
					shaderGroupIndicesPerType[base::ShaderGroupHandleType::MISS].push_back(idx);
					missCount++;
				}
				else if (vkStage == VK_SHADER_STAGE_CALLABLE_BIT_KHR)
				{
					shaderGroupIndicesPerType[base::ShaderGroupHandleType::CALLABLE].push_back(idx);
					callableCount++;
				}
				else
					continue;
			}
			else
			{
				shaderGroupIndicesPerType[base::ShaderGroupHandleType::HIT_GROUP].push_back(idx);
				hitCount++;
			}
			idx++;
		}

		//Allocate new memory for the handles to be copied into.
		m_ShaderGroupHandles.reserve(idx);

		//Copy Shader handles to the new memory in order.
		for (size_t type = 0; type < 4; type++)
		{
			for (size_t& shaderGroupIndexPerType : shaderGroupIndicesPerType[base::ShaderGroupHandleType(type)])
			{
				m_ShaderGroupHandles.push_back({});
				m_ShaderGroupHandles.back().first = base::ShaderGroupHandleType(type);
				m_ShaderGroupHandles.back().second.resize(handleSize);
				memcpy_s(m_ShaderGroupHandles.back().second.data(), handleSize,
					&shaderGroupHandles[shaderGroupIndexPerType * handleSize], handleSize);
			}
		}
	}	
	else
		MIRU_ASSERT(true, "ERROR: VULKAN: Unknown pipeline type.");
}

std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> Pipeline::GetShaderGroupHandles()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if(m_CI.type == base::PipelineType::RAY_TRACING)
	{
		return m_ShaderGroupHandles;
	}
	else
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: Pipeline type is not RAY_TRACING. Unable to get ShaderGroupHandles.");
	}
	return std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>>();
}

Pipeline::~Pipeline()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
}

VkFormat Pipeline::ToVkFormat(base::VertexType type)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (type)
	{
	case base::VertexType::FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case base::VertexType::VEC2:
		return VK_FORMAT_R32G32_SFLOAT;
	case base::VertexType::VEC3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case base::VertexType::VEC4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case base::VertexType::INT:
		return VK_FORMAT_R32_SINT;
	case base::VertexType::IVEC2:
		return VK_FORMAT_R32G32_SINT;
	case base::VertexType::IVEC3:
		return VK_FORMAT_R32G32B32_SINT;
	case base::VertexType::IVEC4:
		return VK_FORMAT_R32G32B32A32_SINT;
	case base::VertexType::UINT:
		return VK_FORMAT_R32_UINT;
	case base::VertexType::UVEC2:
		return VK_FORMAT_R32G32_UINT;
	case base::VertexType::UVEC3:
		return VK_FORMAT_R32G32B32_UINT;
	case base::VertexType::UVEC4:
		return VK_FORMAT_R32G32B32A32_UINT;
	case base::VertexType::DOUBLE:
		return VK_FORMAT_R64_SFLOAT;
	case base::VertexType::DVEC2:
		return VK_FORMAT_R64G64_SFLOAT;
	case base::VertexType::DVEC3:
		return VK_FORMAT_R64G64B64_SFLOAT;
	case base::VertexType::DVEC4:
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}
#endif