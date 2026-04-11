#pragma once

#include "miru_core_common.h"
#include "PipelineHelper.h"
#include "Shader.h"
#include "Image.h"
#include "DescriptorPoolSet.h"
#include "Sync.h"

namespace miru
{
namespace base
{
	class MIRU_API Pipeline
	{
		//enums/structs
	public:

		struct VertexInputState
		{
			std::vector<VertexInputBindingDescription>		vertexInputBindingDescriptions; //One per VertexBuffer
			std::vector<VertexInputAttributeDescription>	vertexInputAttributeDescriptions; //One per attribute per VertexBuffer
		};
		struct InputAssemblyState 
		{
			PrimitiveTopology	topology;
			bool				primitiveRestartEnable;
		};
		struct TessellationState 
		{
			uint32_t patchControlPoints;
		};
		struct ViewportState
		{
			std::vector<Viewport>	viewports;
			std::vector<Rect2D>		scissors;
		};
		struct RasterisationState
		{
			bool			depthClampEnable;
			bool			rasteriserDiscardEnable;
			PolygonMode		polygonMode;
			CullModeBit		cullMode;
			FrontFace		frontFace;
			bool			depthBiasEnable;
			float			depthBiasConstantFactor;
			float			depthBiasClamp;
			float			depthBiasSlopeFactor;
			float			lineWidth;
		};
		struct MultisampleState
		{
			Image::SampleCountBit	rasterisationSamples;
			bool					sampleShadingEnable;
			float					minSampleShading;
			uint32_t				sampleMask;
			bool					alphaToCoverageEnable;
			bool					alphaToOneEnable;
		};
		struct DepthStencilState
		{
			bool				depthTestEnable;
			bool				depthWriteEnable;
			CompareOp			depthCompareOp;
			bool				depthBoundsTestEnable;
			bool				stencilTestEnable;
			StencilOpState		front;
			StencilOpState		back;
			float				minDepthBounds;
			float				maxDepthBounds;
		};
		struct ColourBlendState
		{
			bool									logicOpEnable;
			LogicOp									logicOp;
			std::vector<ColourBlendAttachmentState>	attachments;
			float									blendConstants[4];
		};
		struct DynamicStates
		{
			std::vector<DynamicState> dynamicStates;
		};
		struct PipelineLayout
		{
			std::vector<DescriptorSetLayoutRef>	descriptorSetLayouts;
			std::vector<PushConstantRange>		pushConstantRanges;
		};

		static constexpr uint32_t ShaderUnused = ~0;
		struct ShaderGroupInfo
		{
			ShaderGroupType type;								//If GENERAL, specify only a generalShader. If TRIANGLES_HIT_GROUP, specify a closestHitShader and/or an anyHitShader. If PROCEDURAL_HIT_GROUP, specify a closestHitShader and/or an anyHitShader and an intersectionShader must be specified.
			uint32_t		generalShader;						//Index into a contiguous array cross of all CreateInfo::shaders::stageAndEntryPoints for either a Raygen, Miss or Callable shader.
			uint32_t		anyHitShader;						//Index into a contiguous array cross of all CreateInfo::shaders::stageAndEntryPoints for the Any Hit shader.
			uint32_t		closestHitShader;					//Index into a contiguous array cross of all CreateInfo::shaders::stageAndEntryPoints for the Closest Hit shader.
			uint32_t		intersectionShader;					//Index into a contiguous array cross of all CreateInfo::shaders::stageAndEntryPoints for the Intersection Shader.
			PipelineLayout	layout;								//Needed for D3D12 DXR Local Root Signatures.
			uint32_t		layoutDescriptorSetNumOffset = 0;	//Needed for D3D12 DXR Local Root Signatures.
		};
		struct RayTracingInfo
		{
			uint32_t		maxRecursionDepth;
			uint32_t		maxPayloadSize;
			uint32_t		maxHitAttributeSize;
			AllocatorRef	allocator;							//Needed for allocating SBT buffers. Allocator::CreateInfo::properties must be Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT.
		};

		struct DynamicRendering
		{
			uint32_t					viewMask = 0;
			std::vector<Image::Format>	colourAttachmentFormats;
			Image::Format				depthAttachmentFormat;
			Image::Format				stencilAttachmentFormat;
		};
		
		struct CreateInfo
		{
			std::string						debugName;
			DeviceRef						device;
			PipelineType					type;
			std::vector<ShaderRef>			shaders;			//One shader only for compute; multiple for Graphics and Ray Tracing.
			VertexInputState				vertexInputState;	//Graphics only.
			InputAssemblyState				inputAssemblyState;	//Graphics only.
			TessellationState				tessellationState;	//Graphics only.
			ViewportState					viewportState;		//Graphics only.
			RasterisationState				rasterisationState;	//Graphics only.
			MultisampleState				multisampleState;	//Graphics only.
			DepthStencilState				depthStencilState;	//Graphics only.
			ColourBlendState				colourBlendState;	//Graphics only.
			DynamicStates					dynamicStates;		//Graphics and Ray Tracing only.
			DynamicRendering				dynamicRendering;	//Graphics only.
			std::vector<ShaderGroupInfo>	shaderGroupInfos;	//Ray Tracing only.
			RayTracingInfo					rayTracingInfo;		//Ray Tracing only.
			PipelineLayout					layout;				//All.
		};

		//Methods
	public:
		static PipelineRef Create(CreateInfo* pCreateInfo);
		virtual ~Pipeline() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual std::vector<std::pair<ShaderGroupHandleType, std::vector<uint8_t>>> GetShaderGroupHandles() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	struct RenderingAttachmentInfo
	{
		ImageViewRef		imageView;
		Image::Layout		imageLayout;
		ResolveModeBits		resolveMode;
		ImageViewRef		resolveImageView;
		Image::Layout		resolveImageLayout;
		AttachmentLoadOp	loadOp;
		AttachmentStoreOp	storeOp;
		Image::ClearValue	clearValue;
	};
	struct RenderingInfo
	{
		RenderingFlagBits						flags;
		Rect2D									renderArea;
		uint32_t								layerCount;
		uint32_t								viewMask;			//Used for multiview rendering.
		std::vector<RenderingAttachmentInfo>	colourAttachments;
		RenderingAttachmentInfo*				pDepthAttachment;
		RenderingAttachmentInfo*				pStencilAttachment;
	};
}
}
