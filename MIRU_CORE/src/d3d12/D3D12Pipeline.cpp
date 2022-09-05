#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Pipeline.h"
#include "D3D12DescriptorPoolSet.h"
#include "D3D12Shader.h"
#include "D3D12Image.h"
#include "D3D12Buffer.h"

using namespace miru;
using namespace d3d12;

//RenderPass
RenderPass::RenderPass(RenderPass::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
}

RenderPass::~RenderPass()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

//Pipeline
Pipeline::Pipeline(Pipeline::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_GlobalRootSignature = CreateRootSignature(m_CI.layout);

	if (m_CI.type == base::PipelineType::GRAPHICS)
	{
		//ShaderStages
		for (auto& shader : m_CI.shaders)
		{
			switch (shader->GetCreateInfo().stageAndEntryPoints[0].first)
			{
			case Shader::StageBit::VERTEX_BIT:
				m_GPSD.VS = ref_cast<Shader>(shader)->m_ShaderByteCode; continue;
			case Shader::StageBit::PIXEL_BIT:
				m_GPSD.PS = ref_cast<Shader>(shader)->m_ShaderByteCode; continue;
			case Shader::StageBit::DOMAIN_BIT:
				m_GPSD.DS = ref_cast<Shader>(shader)->m_ShaderByteCode; continue;
			case Shader::StageBit::HULL_BIT:
				m_GPSD.HS = ref_cast<Shader>(shader)->m_ShaderByteCode; continue;
			case Shader::StageBit::GEOMETRY_BIT:
				m_GPSD.GS = ref_cast<Shader>(shader)->m_ShaderByteCode; continue;
			default:
				continue;
			}
		}

		//VertexInput
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
		for (auto& attrib : m_CI.vertexInputState.vertexInputAttributeDescriptions)
		{
			D3D12_INPUT_ELEMENT_DESC il;
			il.SemanticName = attrib.semanticName.c_str();
			il.SemanticIndex = attrib.location;
			il.Format = ToDXGI_FORMAT(attrib.vertexType);
			il.InputSlot = attrib.binding;
			il.AlignedByteOffset = attrib.offset;
			il.InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(m_CI.vertexInputState.vertexInputBindingDescriptions[attrib.binding].inputRate);
			il.InstanceDataStepRate = static_cast<UINT>(m_CI.vertexInputState.vertexInputBindingDescriptions[attrib.binding].inputRate);
			inputLayout.push_back(il);
		}
		m_GPSD.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };

		//InputAssembly
		switch (m_CI.inputAssemblyState.topology)
		{
		case base::PrimitiveTopology::POINT_LIST:
			m_GPSD.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
		case base::PrimitiveTopology::LINE_LIST:
		case base::PrimitiveTopology::LINE_STRIP:
		case base::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
		case base::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
			m_GPSD.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
		case base::PrimitiveTopology::TRIANGLE_LIST:
		case base::PrimitiveTopology::TRIANGLE_STRIP:
		case base::PrimitiveTopology::TRIANGLE_FAN:
		case base::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
		case base::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
			m_GPSD.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
		case base::PrimitiveTopology::PATCH_LIST:
			m_GPSD.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH; break;
		default:
			m_GPSD.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED; break;
		}

		//Tessellation
		m_GPSD.StreamOutput = {};

		//Viewport
		for (auto& viewport : m_CI.viewportState.viewports)
			m_Viewports.push_back({ viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth });
		for (auto& scissor : m_CI.viewportState.scissors)
			m_Scissors.push_back({ static_cast<LONG>(scissor.offset.x), static_cast<LONG>(scissor.offset.y), static_cast<LONG>(scissor.extent.width), static_cast<LONG>(scissor.extent.height) });

		//Rasterisation
		m_GPSD.RasterizerState.FillMode = m_CI.rasterisationState.polygonMode == base::PolygonMode::LINE ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
		m_GPSD.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(static_cast<uint32_t>(m_CI.rasterisationState.cullMode) % 3 + 1); //%3 because d3d12 has no FRONT_AND_BACK
		m_GPSD.RasterizerState.FrontCounterClockwise = !static_cast<bool>(m_CI.rasterisationState.frontFace);
		m_GPSD.RasterizerState.DepthBias = static_cast<INT>(m_CI.rasterisationState.depthBiasConstantFactor);
		m_GPSD.RasterizerState.DepthBiasClamp = m_CI.rasterisationState.depthBiasClamp;
		m_GPSD.RasterizerState.SlopeScaledDepthBias = m_CI.rasterisationState.depthBiasSlopeFactor;
		m_GPSD.RasterizerState.DepthClipEnable = m_CI.rasterisationState.depthClampEnable;
		m_GPSD.RasterizerState.MultisampleEnable = m_CI.multisampleState.rasterisationSamples > base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT; //Sets AA algorithm
		m_GPSD.RasterizerState.AntialiasedLineEnable = m_CI.multisampleState.rasterisationSamples > base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;//Sets AA algorithm
		m_GPSD.RasterizerState.ForcedSampleCount = 0;
		m_GPSD.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//Multisample
		m_GPSD.SampleDesc.Count = static_cast<UINT>(m_CI.multisampleState.rasterisationSamples);
		m_GPSD.SampleDesc.Quality = 0;
		m_GPSD.SampleMask = static_cast<UINT>(m_CI.multisampleState.sampleMask);

		//DepthStencil
		m_GPSD.DepthStencilState.DepthEnable = m_CI.depthStencilState.depthTestEnable;
		m_GPSD.DepthStencilState.DepthWriteMask = static_cast<D3D12_DEPTH_WRITE_MASK>(m_CI.depthStencilState.depthWriteEnable);
		m_GPSD.DepthStencilState.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(static_cast<uint32_t>(m_CI.depthStencilState.depthCompareOp) + 1);
		m_GPSD.DepthStencilState.StencilEnable = m_CI.depthStencilState.stencilTestEnable;
		m_GPSD.DepthStencilState.StencilReadMask = shrink_uint32_t_to_uint8_t(m_GPSD.RasterizerState.CullMode == D3D12_CULL_MODE_FRONT ? m_CI.depthStencilState.back.compareMask : m_CI.depthStencilState.front.compareMask);
		m_GPSD.DepthStencilState.StencilWriteMask = shrink_uint32_t_to_uint8_t(m_GPSD.RasterizerState.CullMode == D3D12_CULL_MODE_FRONT ? m_CI.depthStencilState.back.writeMask : m_CI.depthStencilState.front.writeMask);
		m_GPSD.DepthStencilState.FrontFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(static_cast<uint32_t>(m_CI.depthStencilState.front.failOp) + 1);
		m_GPSD.DepthStencilState.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(static_cast<uint32_t>(m_CI.depthStencilState.front.depthFailOp) + 1);
		m_GPSD.DepthStencilState.FrontFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(static_cast<uint32_t>(m_CI.depthStencilState.front.passOp) + 1);
		m_GPSD.DepthStencilState.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(static_cast<uint32_t>(m_CI.depthStencilState.front.compareOp) + 1);
		m_GPSD.DepthStencilState.BackFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(static_cast<uint32_t>(m_CI.depthStencilState.back.failOp) + 1);
		m_GPSD.DepthStencilState.BackFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(static_cast<uint32_t>(m_CI.depthStencilState.back.depthFailOp) + 1);
		m_GPSD.DepthStencilState.BackFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(static_cast<uint32_t>(m_CI.depthStencilState.back.passOp) + 1);
		m_GPSD.DepthStencilState.BackFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(static_cast<uint32_t>(m_CI.depthStencilState.back.compareOp) + 1);

		//ColourBlend
		m_GPSD.BlendState.AlphaToCoverageEnable = m_CI.multisampleState.alphaToCoverageEnable;
		m_GPSD.BlendState.IndependentBlendEnable = true;
		size_t i = 0;
		for (auto& blend : m_CI.colourBlendState.attachments)
		{
			m_GPSD.BlendState.RenderTarget[i].BlendEnable = blend.blendEnable;
			m_GPSD.BlendState.RenderTarget[i].LogicOpEnable = m_CI.colourBlendState.logicOpEnable;
			m_GPSD.BlendState.RenderTarget[i].SrcBlend = ToD3D12_BLEND(blend.srcColourBlendFactor);
			m_GPSD.BlendState.RenderTarget[i].DestBlend = ToD3D12_BLEND(blend.dstColourBlendFactor);
			m_GPSD.BlendState.RenderTarget[i].BlendOp = static_cast<D3D12_BLEND_OP>(static_cast<uint32_t>(blend.colourBlendOp) + 1);
			m_GPSD.BlendState.RenderTarget[i].SrcBlendAlpha = ToD3D12_BLEND(blend.srcAlphaBlendFactor);
			m_GPSD.BlendState.RenderTarget[i].DestBlendAlpha = ToD3D12_BLEND(blend.dstAlphaBlendFactor);
			m_GPSD.BlendState.RenderTarget[i].BlendOpAlpha = static_cast<D3D12_BLEND_OP>(static_cast<uint32_t>(blend.alphaBlendOp) + 1);
			m_GPSD.BlendState.RenderTarget[i].LogicOp = ToD3D12_LOGIC_OP(m_CI.colourBlendState.logicOp);
			m_GPSD.BlendState.RenderTarget[i].RenderTargetWriteMask = static_cast<UINT8>(blend.colourWriteMask);

			i++;
			if (i >= 8)
				break;
		}

		//Dynamic

		//RTV and DSV
		if (m_CI.renderPass)
		{
			size_t j = 0;
			for (auto& attachment : m_CI.renderPass->GetCreateInfo().subpassDescriptions[m_CI.subpassIndex].colourAttachments)
			{
				if (attachment.layout == Image::Layout::COLOUR_ATTACHMENT_OPTIMAL)
					m_GPSD.RTVFormats[j] = Image::ToD3D12ImageFormat(m_CI.renderPass->GetCreateInfo().attachments[attachment.attachmentIndex].format);

				j++;
				if (j >= 8)
					break;
			}
			m_GPSD.NumRenderTargets = static_cast<UINT>(j);
			for (auto& attachment : m_CI.renderPass->GetCreateInfo().subpassDescriptions[m_CI.subpassIndex].depthStencilAttachment)
			{
				if (attachment.layout == Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					|| attachment.layout == Image::Layout::DEPTH_STENCIL_READ_ONLY_OPTIMAL
					|| attachment.layout == Image::Layout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
					|| attachment.layout == Image::Layout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
					|| attachment.layout == Image::Layout::DEPTH_ATTACHMENT_OPTIMAL
					|| attachment.layout == Image::Layout::DEPTH_READ_ONLY_OPTIMAL
					|| attachment.layout == Image::Layout::STENCIL_ATTACHMENT_OPTIMAL
					|| attachment.layout == Image::Layout::STENCIL_READ_ONLY_OPTIMAL)
				{
					m_GPSD.DSVFormat = Image::ToD3D12ImageFormat(m_CI.renderPass->GetCreateInfo().attachments[attachment.attachmentIndex].format);
				}

				break; //There can be only one DSV.
			}
			if (m_GPSD.DSVFormat == DXGI_FORMAT_UNKNOWN) //If no DSV, then DepthStencilState must be null.
				m_GPSD.DepthStencilState = {};
		}
		else
		{
			size_t j = 0;
			for (auto& colourAttachmentFormat : m_CI.dynamicRendering.colourAttachmentFormats)
			{
				m_GPSD.RTVFormats[j] = Image::ToD3D12ImageFormat(colourAttachmentFormat);
				
				j++;
				if (j >= 8)
					break;
			}
			m_GPSD.NumRenderTargets = static_cast<UINT>(j);
			m_GPSD.DSVFormat = Image::ToD3D12ImageFormat(m_CI.dynamicRendering.depthAttachmentFormat);

			if (m_GPSD.DSVFormat == DXGI_FORMAT_UNKNOWN) //If no DSV, then DepthStencilState must be null.
				m_GPSD.DepthStencilState = {};
		}

		//Fill D3D12 structure
		m_GPSD.pRootSignature = m_GlobalRootSignature.rootSignature;
		m_GPSD.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		m_GPSD.NodeMask = 0;
		m_GPSD.CachedPSO = {};
		m_GPSD.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		CD3DX12_PIPELINE_STATE_STREAM2 gpss2(m_GPSD);

		if (m_CI.renderPass && !m_CI.renderPass->GetCreateInfo().multiview.viewMasks.empty())
		{
			const RenderPass::Multiview& multiview = m_CI.renderPass->GetCreateInfo().multiview;
			m_ViewInstancingDesc.ViewInstanceCount = std::bit_width(multiview.viewMasks[static_cast<size_t>(m_CI.subpassIndex)]);
			m_ViewInstanceLocations.resize(m_ViewInstancingDesc.ViewInstanceCount);
			for (size_t i = 0; i < m_ViewInstanceLocations.size(); i++)
			{
				m_ViewInstanceLocations[i].RenderTargetArrayIndex = 0; //static_cast<UINT>(i);
				m_ViewInstanceLocations[i].ViewportArrayIndex = 0;
			}
			m_ViewInstancingDesc.pViewInstanceLocations = m_ViewInstanceLocations.data();
			m_ViewInstancingDesc.Flags = D3D12_VIEW_INSTANCING_FLAG_ENABLE_VIEW_INSTANCE_MASKING;

			gpss2.ViewInstancingDesc = CD3DX12_VIEW_INSTANCING_DESC(m_ViewInstancingDesc);
		}
		
		D3D12_PIPELINE_STATE_STREAM_DESC gpssd;
		gpssd.SizeInBytes = sizeof(gpss2);
		gpssd.pPipelineStateSubobjectStream = &gpss2;

		MIRU_ASSERT(reinterpret_cast<ID3D12Device2*>(m_Device)->CreatePipelineState(&gpssd, IID_PPV_ARGS(&m_Pipeline)), "ERROR: D3D12: Failed to create Graphics Pipeline.");
		D3D12SetName(m_Pipeline, m_CI.debugName + " : Graphics Pipeline");
	}
	else if (m_CI.type == base::PipelineType::COMPUTE)
	{
		m_CPSD.pRootSignature = m_GlobalRootSignature.rootSignature;
		m_CPSD.CS = ref_cast<Shader>(m_CI.shaders[0])->m_ShaderByteCode;
		m_CPSD.NodeMask = 0;
		m_CPSD.CachedPSO = {};
		m_CPSD.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		CD3DX12_PIPELINE_STATE_STREAM2 cpss2(m_CPSD);

		D3D12_PIPELINE_STATE_STREAM_DESC cpssd;
		cpssd.SizeInBytes = sizeof(cpss2);
		cpssd.pPipelineStateSubobjectStream = &cpss2;

		MIRU_ASSERT(reinterpret_cast<ID3D12Device2*>(m_Device)->CreatePipelineState(&cpssd, IID_PPV_ARGS(&m_Pipeline)), "ERROR: D3D12: Failed to create Compute Pipeline.");
		D3D12SetName(m_Pipeline, m_CI.debugName + " : Compute Pipeline");
	}
	else if (m_CI.type == base::PipelineType::RAY_TRACING)
	{
		size_t totalShaderCount = 0;
		for (auto& shader : m_CI.shaders)
			for (auto& stageAndEntryPoint : shader->GetCreateInfo().stageAndEntryPoints)
				totalShaderCount++;

		//Resevere enough D3D12_STATE_SUBOBJECTs: All Library shader + (HitGroup + LocalRootSignature + ExportsAssociation) + (GlobalRootSignature + ShaderConfig + PipelineConfig)
		m_RayTracingPipelineSubDesc.reserve(m_CI.shaders.size() + 3 * m_CI.shaderGroupInfos.size() + size_t(3));

		//Fill out DXIL Library and Exports
		std::vector<D3D12_DXIL_LIBRARY_DESC> dxilLibDescs;
		std::vector<D3D12_EXPORT_DESC> exportDescs;
		std::vector<std::pair<base::Shader::StageBit, std::wstring>> exportStagesAndNames;
		dxilLibDescs.reserve(m_CI.shaders.size());
		exportDescs.reserve(totalShaderCount);
		exportStagesAndNames.reserve(totalShaderCount);

		for (auto& shader : m_CI.shaders)
		{
			UINT exportCount = 0;
			for (auto& stageAndEntryPoint : shader->GetCreateInfo().stageAndEntryPoints)
			{
				D3D12_EXPORT_DESC exportDesc;
				exportStagesAndNames.push_back({ stageAndEntryPoint.first, arc::ToWString(stageAndEntryPoint.second) });
				exportDesc.Name = exportStagesAndNames.back().second.c_str();
				exportDesc.ExportToRename = nullptr;
				exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
				exportDescs.push_back(exportDesc);
				exportCount++;
			}

			D3D12_DXIL_LIBRARY_DESC dxilLibDesc;
			dxilLibDesc.DXILLibrary = ref_cast<Shader>(shader)->m_ShaderByteCode;
			dxilLibDesc.NumExports = exportCount;
			dxilLibDesc.pExports = &exportDescs[exportDescs.size() - static_cast<size_t>(exportCount)];
			dxilLibDescs.push_back(dxilLibDesc);
			D3D12_STATE_SUBOBJECT dxilLibrary;
			dxilLibrary.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			dxilLibrary.pDesc = &dxilLibDescs.back();
			m_RayTracingPipelineSubDesc.push_back(dxilLibrary);
		}

		//Fill out HitGroups
		std::vector<D3D12_HIT_GROUP_DESC> hitGroupDescs;
		std::vector<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION> subobjectToExportAssociations;
		std::vector<std::wstring> hitGroupNames;
		hitGroupNames.reserve(m_CI.shaderGroupInfos.size());
		hitGroupDescs.reserve(m_CI.shaderGroupInfos.size());
		subobjectToExportAssociations.reserve(m_CI.shaderGroupInfos.size());
		m_LocalRootSignatures.reserve(m_CI.shaderGroupInfos.size());
		UINT hitGroupCount = 0;
		for (auto& shaderGroupInfo : m_CI.shaderGroupInfos)
		{
			if (shaderGroupInfo.type > base::ShaderGroupType::GENERAL)
			{
				D3D12_HIT_GROUP_DESC hitGroupDesc;
				hitGroupNames.push_back(L"HitGroup_" + std::to_wstring(hitGroupCount));
				hitGroupDesc.HitGroupExport = hitGroupNames.back().c_str();
				hitGroupDesc.Type = static_cast<D3D12_HIT_GROUP_TYPE>(uint32_t(shaderGroupInfo.type) - 1);
				hitGroupDesc.AnyHitShaderImport = nullptr;
				hitGroupDesc.ClosestHitShaderImport = nullptr;
				hitGroupDesc.IntersectionShaderImport = nullptr;
				if (shaderGroupInfo.anyHitShader != ShaderUnused)
					hitGroupDesc.AnyHitShaderImport = exportDescs[shaderGroupInfo.anyHitShader].Name;
				if (shaderGroupInfo.closestHitShader != ShaderUnused)
					hitGroupDesc.ClosestHitShaderImport = exportDescs[shaderGroupInfo.closestHitShader].Name;
				if (shaderGroupInfo.intersectionShader != ShaderUnused)
					hitGroupDesc.IntersectionShaderImport = exportDescs[shaderGroupInfo.intersectionShader].Name;
				hitGroupDescs.push_back(hitGroupDesc);
				hitGroupCount++;

				D3D12_STATE_SUBOBJECT hitGroup;
				hitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
				hitGroup.pDesc = &hitGroupDescs.back();
				m_RayTracingPipelineSubDesc.push_back(hitGroup);
			}

			//Deal with any ShaderGroup Parameters
			if (shaderGroupInfo.layout.descriptorSetLayouts.size()
				|| shaderGroupInfo.layout.pushConstantRanges.size())
			{
				m_LocalRootSignatures.emplace_back(std::move(CreateRootSignature(shaderGroupInfo.layout, shaderGroupInfo.layoutDescriptorSetNumOffset, true)));

				//Fill out LocalRootSignature
				D3D12_STATE_SUBOBJECT localRootSignature;
				localRootSignature.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
				localRootSignature.pDesc = &m_LocalRootSignatures.back();
				m_RayTracingPipelineSubDesc.push_back(localRootSignature);

				//Fill out SubobjectsToExportsAssociation
				D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION subobjectToExportAssociation;
				subobjectToExportAssociation.pSubobjectToAssociate = &m_RayTracingPipelineSubDesc.back();
				subobjectToExportAssociation.NumExports = 1;
				if (shaderGroupInfo.type > base::ShaderGroupType::GENERAL)
					subobjectToExportAssociation.pExports = &(hitGroupDescs.back().HitGroupExport);
				else
					subobjectToExportAssociation.pExports = &(exportDescs[shaderGroupInfo.generalShader].Name);
				subobjectToExportAssociations.push_back(subobjectToExportAssociation);

				D3D12_STATE_SUBOBJECT subobjectToExportAssociationSubObject;
				subobjectToExportAssociationSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				subobjectToExportAssociationSubObject.pDesc = &subobjectToExportAssociations.back();
				m_RayTracingPipelineSubDesc.push_back(subobjectToExportAssociationSubObject);
			}
		}

		//Fill out GlobalRootSignature
		D3D12_STATE_SUBOBJECT globalRootSignature;
		globalRootSignature.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		globalRootSignature.pDesc = &m_GlobalRootSignature.rootSignature;
		m_RayTracingPipelineSubDesc.push_back(globalRootSignature);

		//Fill out ShaderConfig
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
		shaderConfig.MaxPayloadSizeInBytes = m_CI.rayTracingInfo.maxPayloadSize;
		shaderConfig.MaxAttributeSizeInBytes = m_CI.rayTracingInfo.maxHitAttributeSize;
		D3D12_STATE_SUBOBJECT shaderConfigSubObject;
		shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigSubObject.pDesc = &shaderConfig;
		m_RayTracingPipelineSubDesc.push_back(shaderConfigSubObject);

		//Fill out PipelineConfig
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
		pipelineConfig.MaxTraceRecursionDepth = m_CI.rayTracingInfo.maxRecursionDepth;
		D3D12_STATE_SUBOBJECT pipelineConfigSubObject;
		pipelineConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipelineConfigSubObject.pDesc = &pipelineConfig;
		m_RayTracingPipelineSubDesc.push_back(pipelineConfigSubObject);

		//Fill out StateObject
		m_RayTracingPipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		m_RayTracingPipelineDesc.NumSubobjects = static_cast<UINT>(m_RayTracingPipelineSubDesc.size());
		m_RayTracingPipelineDesc.pSubobjects = m_RayTracingPipelineSubDesc.data();

		MIRU_ASSERT(reinterpret_cast<ID3D12Device5*>(m_Device)->CreateStateObject(&m_RayTracingPipelineDesc, IID_PPV_ARGS(&m_RayTracingPipeline)), "ERROR: D3D12: Failed to create Ray Tracing Pipeline.");
		D3D12SetName(m_RayTracingPipeline, m_CI.debugName + " : Ray Tracing Pipeline");

		// Get ShaderGroupIdentifiers
		ID3D12StateObjectProperties * rayTracingPipelineProperties = nullptr;
		MIRU_ASSERT(m_RayTracingPipeline->QueryInterface(IID_PPV_ARGS(&rayTracingPipelineProperties)), "ERROR: D3D12: Failed to get Ray Tracing Pipeline Properties.");

		std::map<base::ShaderGroupHandleType, std::vector<void*>>shaderIdentifiers;
		for (auto& exportStageAndName : exportStagesAndNames)
		{
			base::Shader::StageBit stage = exportStageAndName.first;
			std::wstring name = exportStageAndName.second;

			if (stage == base::Shader::StageBit::RAYGEN_BIT)
				shaderIdentifiers[base::ShaderGroupHandleType::RAYGEN].push_back(rayTracingPipelineProperties->GetShaderIdentifier(name.c_str()));
			else if (stage == base::Shader::StageBit::MISS_BIT)
				shaderIdentifiers[base::ShaderGroupHandleType::MISS].push_back(rayTracingPipelineProperties->GetShaderIdentifier(name.c_str()));
			else if (stage == base::Shader::StageBit::CALLABLE_BIT)
				shaderIdentifiers[base::ShaderGroupHandleType::CALLABLE].push_back(rayTracingPipelineProperties->GetShaderIdentifier(name.c_str()));
			else
				continue;
		}
		for (auto& hitGroupName : hitGroupNames)
			shaderIdentifiers[base::ShaderGroupHandleType::HIT_GROUP].push_back(rayTracingPipelineProperties->GetShaderIdentifier(hitGroupName.c_str()));

		//ShaderHandleSize
		const size_t& handleSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		const size_t& handleSizeAligned = alignedSize(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		//Allocate new memory for the handles to be copied into.
		m_ShaderGroupHandles.reserve(shaderIdentifiers.size());

		//Copy Shader handles to the new memory in order.
		for (size_t type = 0; type < 4; type++)
		{
			for (auto& shaderIdentifier : shaderIdentifiers[base::ShaderGroupHandleType(type)])
			{
				m_ShaderGroupHandles.push_back({});
				m_ShaderGroupHandles.back().first = base::ShaderGroupHandleType(type);
				m_ShaderGroupHandles.back().second.resize(handleSize);
				memcpy_s(m_ShaderGroupHandles.back().second.data(), handleSize,
					shaderIdentifier, handleSize);
			}
		}
	}
	else
		MIRU_ASSERT(true, "ERROR: D3D12: Unknown pipeline type.");
}

std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> Pipeline::GetShaderGroupHandles()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_CI.type == base::PipelineType::RAY_TRACING)
	{
		return m_ShaderGroupHandles;
	}
	else
	{
		MIRU_ASSERT(true, "ERROR: D3D12: Pipeline type is not RAY_TRACING. Unable to get ShaderGroupHandles.");
	}
	return std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>>();
}

Pipeline::~Pipeline()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_Pipeline);
	MIRU_D3D12_SAFE_RELEASE(m_RayTracingPipeline);
	MIRU_D3D12_SAFE_RELEASE(m_GlobalRootSignature.rootSignature);
	MIRU_D3D12_SAFE_RELEASE(m_GlobalRootSignature.serializedRootSignature);
	MIRU_D3D12_SAFE_RELEASE(m_GlobalRootSignature.serializedRootSignatureError);

	for (auto& localRootSignature : m_LocalRootSignatures)
	{
		MIRU_D3D12_SAFE_RELEASE(localRootSignature.rootSignature);
		MIRU_D3D12_SAFE_RELEASE(localRootSignature.serializedRootSignature);
		MIRU_D3D12_SAFE_RELEASE(localRootSignature.serializedRootSignatureError);
	}
}

D3D12_PRIMITIVE_TOPOLOGY Pipeline::ToD3D12_PRIMITIVE_TOPOLOGY(base::PrimitiveTopology topology)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch(topology)
	{
	case base::PrimitiveTopology::POINT_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case base::PrimitiveTopology::LINE_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case base::PrimitiveTopology::LINE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case base::PrimitiveTopology::TRIANGLE_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case base::PrimitiveTopology::TRIANGLE_STRIP:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case base::PrimitiveTopology::TRIANGLE_FAN:
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	case base::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
		return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
	case base::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
	case base::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
	case base::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
	case base::PrimitiveTopology::PATCH_LIST:
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	default:
		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

DXGI_FORMAT Pipeline::ToDXGI_FORMAT(base::VertexType type)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (type)
	{
	case base::VertexType::FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case base::VertexType::VEC2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case base::VertexType::VEC3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case base::VertexType::VEC4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case base::VertexType::INT:
		return DXGI_FORMAT_R32_SINT;
	case base::VertexType::IVEC2:
		return DXGI_FORMAT_R32G32_SINT;
	case base::VertexType::IVEC3:
		return DXGI_FORMAT_R32G32B32_SINT;
	case base::VertexType::IVEC4:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case base::VertexType::UINT:
		return DXGI_FORMAT_R32_UINT;
	case base::VertexType::UVEC2:
		return DXGI_FORMAT_R32G32_UINT;
	case base::VertexType::UVEC3:
		return DXGI_FORMAT_R32G32B32_UINT;
	case base::VertexType::UVEC4:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case base::VertexType::DOUBLE:
		return DXGI_FORMAT_R32_FLOAT;
	case base::VertexType::DVEC2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case base::VertexType::DVEC3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case base::VertexType::DVEC4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_BLEND Pipeline::ToD3D12_BLEND(base::BlendFactor blend)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (blend)
	{
	default:
	case base::BlendFactor::ZERO:
		return D3D12_BLEND_ZERO;
	case base::BlendFactor::ONE:
		return D3D12_BLEND_ONE;
	case base::BlendFactor::SRC_COLOUR:
		return D3D12_BLEND_SRC_COLOR;
	case base::BlendFactor::ONE_MINUS_SRC_COLOUR:
		return D3D12_BLEND_INV_SRC_COLOR;
	case base::BlendFactor::DST_COLOUR:
		return D3D12_BLEND_DEST_COLOR;
	case base::BlendFactor::ONE_MINUS_DST_COLOUR:
		return D3D12_BLEND_INV_DEST_COLOR;
	case base::BlendFactor::SRC_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case base::BlendFactor::ONE_MINUS_SRC_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case base::BlendFactor::DST_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case base::BlendFactor::ONE_MINUS_DST_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case base::BlendFactor::CONSTANT_COLOUR:
		return D3D12_BLEND_BLEND_FACTOR;
	case base::BlendFactor::ONE_MINUS_CONSTANT_COLOUR:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	case base::BlendFactor::CONSTANT_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case base::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case base::BlendFactor::SRC_ALPHA_SATURATE:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case base::BlendFactor::SRC1_COLOUR:
		return D3D12_BLEND_SRC1_COLOR;
	case base::BlendFactor::ONE_MINUS_SRC1_COLOUR:
		return D3D12_BLEND_INV_SRC1_COLOR;
	case base::BlendFactor::SRC1_ALPHA:
		return D3D12_BLEND_SRC1_ALPHA;
	case base::BlendFactor::ONE_MINUS_SRC1_ALPHA:
		return D3D12_BLEND_INV_SRC1_ALPHA;
	}
}

D3D12_LOGIC_OP Pipeline::ToD3D12_LOGIC_OP(base::LogicOp logic)
{
	MIRU_CPU_PROFILE_FUNCTION();

	switch (logic)
	{
	default:
	case base::LogicOp::CLEAR:
		return D3D12_LOGIC_OP_CLEAR;
	case base::LogicOp::AND:
		return D3D12_LOGIC_OP_AND;
	case base::LogicOp::AND_REVERSE:
		return D3D12_LOGIC_OP_AND_REVERSE;
	case base::LogicOp::COPY:
		return D3D12_LOGIC_OP_COPY;
	case base::LogicOp::AND_INVERTED:
		return D3D12_LOGIC_OP_AND_INVERTED;
	case base::LogicOp::NO_OP:
		return D3D12_LOGIC_OP_NOOP;
	case base::LogicOp::XOR:
		return D3D12_LOGIC_OP_XOR;
	case base::LogicOp::OR:
		return D3D12_LOGIC_OP_OR;
	case base::LogicOp::NOR:
		return D3D12_LOGIC_OP_NOR;
	case base::LogicOp::EQUIVALENT:
		return D3D12_LOGIC_OP_EQUIV;
	case base::LogicOp::INVERT:
		return D3D12_LOGIC_OP_INVERT;
	case base::LogicOp::OR_REVERSE:
		return D3D12_LOGIC_OP_OR_REVERSE;
	case base::LogicOp::COPY_INVERTED:
		return D3D12_LOGIC_OP_COPY_INVERTED;
	case base::LogicOp::OR_INVERTED:
		return D3D12_LOGIC_OP_OR_INVERTED;
	case base::LogicOp::NAND:
		return D3D12_LOGIC_OP_NAND;
	case base::LogicOp::SET:
		return D3D12_LOGIC_OP_SET;
	}
}

Pipeline::RootSignature Pipeline::CreateRootSignature(const base::Pipeline::PipelineLayout layout, uint32_t setNumOffset, bool localRootSignature)
{
	RootSignature result;
	result.rootParameters.clear();

	D3D12_ROOT_DESCRIPTOR_TABLE descriptoTableSRV_UAV_CBV;
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTableSampler;

	UINT set = 0;
	set += setNumOffset;
	for (auto& descriptorSetLayout : layout.descriptorSetLayouts)
	{
		descriptoTableSRV_UAV_CBV = {};
		descriptorTableSampler = {};
		result.descriptorRangesSRV_UAV_CBV.push_back({});
		result.descriptorRangesSampler.push_back({});

		for (size_t i = 0; i < 4; i++)
		{
			D3D12_DESCRIPTOR_RANGE descRange = ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorRanges[i];
			if (descRange.NumDescriptors > 0)
			{
				descRange.RegisterSpace = set;
				if (descRange.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
					result.descriptorRangesSampler.back().push_back(descRange);
				else
					result.descriptorRangesSRV_UAV_CBV.back().push_back(descRange);
			}
		}

		std::sort(result.descriptorRangesSRV_UAV_CBV.back().begin(), result.descriptorRangesSRV_UAV_CBV.back().end(),
			[](const D3D12_DESCRIPTOR_RANGE& a, const D3D12_DESCRIPTOR_RANGE& b)
			{
				return a.BaseShaderRegister < b.BaseShaderRegister;
			});

		descriptoTableSRV_UAV_CBV.NumDescriptorRanges = static_cast<UINT>(result.descriptorRangesSRV_UAV_CBV.back().size());
		descriptoTableSRV_UAV_CBV.pDescriptorRanges = result.descriptorRangesSRV_UAV_CBV.back().data();
		if (ref_cast<DescriptorSetLayout>(descriptorSetLayout)->m_DescriptorRanges[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER].NumDescriptors)
		{
			descriptorTableSampler.NumDescriptorRanges = static_cast<UINT>(result.descriptorRangesSampler.back().size());
			descriptorTableSampler.pDescriptorRanges = result.descriptorRangesSampler.back().data();
		}

		D3D12_ROOT_PARAMETER rootParameter;
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = descriptoTableSRV_UAV_CBV;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		result.rootParameters.push_back(rootParameter);
		if (descriptorTableSampler.pDescriptorRanges)
		{
			rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter.DescriptorTable = descriptorTableSampler;
			rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			result.rootParameters.push_back(rootParameter);
		}

		set++;
	}

	for (auto& pushConstantRange : m_CI.layout.pushConstantRanges)
	{
		//TODO: Review this.
		D3D12_ROOT_PARAMETER rootParameter;
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter.Constants.ShaderRegister = 0;
		rootParameter.Constants.RegisterSpace = 0;
		rootParameter.Constants.Num32BitValues = pushConstantRange.size;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		result.rootParameters.push_back(rootParameter);
	}

	result.rootSignatureDesc.NumParameters = static_cast<UINT>(result.rootParameters.size());
	result.rootSignatureDesc.pParameters = result.rootParameters.data();
	result.rootSignatureDesc.NumStaticSamplers = 0;
	result.rootSignatureDesc.pStaticSamplers = nullptr;
	result.rootSignatureDesc.Flags = localRootSignature ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSignatureData;
	rootSignatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	MIRU_ASSERT(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignatureData, sizeof(rootSignatureData)), "ERROR: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_ROOT_SIGNATURE.");

	HRESULT res = D3D12SerializeRootSignature(&result.rootSignatureDesc, rootSignatureData.HighestVersion, &result.serializedRootSignature, &result.serializedRootSignatureError);
	if (result.serializedRootSignatureError)
	{
		std::string errorStr = std::string((char*)result.serializedRootSignatureError->GetBufferPointer());
		MIRU_ERROR(true, "ERROR: D3D12: Error in serialising RootSignature: " + errorStr);
	}
	MIRU_ASSERT(res, "ERROR: D3D12: Failed to serialise RootSignature.");
	MIRU_ASSERT(m_Device->CreateRootSignature(0, result.serializedRootSignature->GetBufferPointer(), result.serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&result.rootSignature)), "ERROR: D3D12: Failed to create RootSignature.");

	return result;
}
#endif