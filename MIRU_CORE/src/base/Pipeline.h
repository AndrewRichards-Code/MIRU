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
	class MIRU_API RenderPass
	{
		//enum/structs
	public:
		enum class AttachmentLoadOp : uint32_t
		{
			LOAD,
			CLEAR,
			DONT_CARE
		};
		enum class AttachmentStoreOp : uint32_t
		{
			STORE,
			DONT_CARE,
			NONE = 1000301000,
		};
		struct AttachmentDescription
		{
			Image::Format			format;
			Image::SampleCountBit	samples;
			AttachmentLoadOp		loadOp;
			AttachmentStoreOp		storeOp;
			AttachmentLoadOp		stencilLoadOp;
			AttachmentStoreOp		stencilStoreOp;
			Image::Layout			initialLayout;	//Layout of Image subresources at the start of all subpasses in this RenderPass.
			Image::Layout			finalLayout;	//Layout of Image subresources at the end of all subpasses in this RenderPass.
		};
		struct AttachmentReference
		{
			uint32_t		attachmentIndex;	//Index into the List of attachments.
			Image::Layout	layout;				//Layout of the attachments in this subpass.
		};
		struct SubpassDescription
		{
			PipelineType						pipelineType;
			std::vector<AttachmentReference>	inputAttachments;		//Attachments to be used as an 'input colour' (Read in per-pixel).
			std::vector<AttachmentReference>	colourAttachments;		//Attachments for the output colour.
			std::vector<AttachmentReference>	resolveAttachments;		//Attachments for the multisample colour attachments to resolve to (if needed), which must relate by index to their respective coloutAttachment.
			std::vector<AttachmentReference>	depthStencilAttachment;	//Attachment to be used the depth-stencil. Only 1 allowed.
			std::vector<AttachmentReference>	preseverseAttachments;	//Attachments that are needed in later subpasses, but not this one.
		};
		struct SubpassDependency
		{
			uint32_t			srcSubpass;		//Index of the Subpass that this subpass is dependent on.
			uint32_t			dstSubpass;		//Index of this subpass.
			PipelineStageBit	srcStage;		//Which stages of the previous subpass need to by completed?
			PipelineStageBit	dstStage;		//Which stages of this subpass are being waitied on?
			Barrier::AccessBit	srcAccess;		//What accesses are needed into the previous subpass stage?
			Barrier::AccessBit	dstAccess;		//What accesses are needed into this subpass stage?
			DependencyBit		dependencies;	//What rendering localities are needed between subpasses?
		};
		static constexpr uint32_t SubpassExternal = ~0;
		struct Multiview
		{
			std::vector<uint32_t>	viewMasks = {};			//Which views rendering is broadcast to in each subpass; described as a bitfield of view indices. size() is 'subpassCount'
			std::vector<int32_t>	viewOffsets = {};		//Which views in the source subpass the views in the destination subpass depend on. size() is 'dependencyCount'
			std::vector<uint32_t>	correlationMasks = {};	//Indicates a sets of views that may be more efficient to render concurrently; described as a bitfield of view indices. size() is 'correlationMaskCount'
		};
		struct CreateInfo
		{
			std::string							debugName;
			void*								device;
			std::vector<AttachmentDescription>	attachments;				//List of a images and their usage throughout the whole RenderPass.
			std::vector<SubpassDescription>		subpassDescriptions;		//List of subpass to executed with the RenderPass.
			std::vector<SubpassDependency>		subpassDependencies;		//List of dependency between subpass. One subpass may have multiple dependcies on other subpasses.
			Multiview							multiview;					//For Multiview RenderPasses only.
		};

		//Methods
	public:
		static RenderPassRef Create(CreateInfo* pCreateInfo);
		virtual ~RenderPass() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

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
			std::vector<DescriptorSetLayoutRef> descriptorSetLayouts;
			std::vector<PushConstantRange>		  pushConstantRanges;
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
			void*							device;
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
			std::vector<ShaderGroupInfo>	shaderGroupInfos;	//Ray Tracing only.
			RayTracingInfo					rayTracingInfo;		//Ray Tracing only.
			PipelineLayout					layout;				//All.
			RenderPassRef					renderPass;			//Graphics only.
			uint32_t						subpassIndex;		//Graphics only.
			DynamicRendering				dynamicRendering;	//Graphics only. Use this if not using a RenderPass.
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
		ImageViewRef					imageView;
		Image::Layout					imageLayout;
		ResolveModeBits					resolveMode;
		ImageViewRef					resolveImageView;
		Image::Layout					resolveImageLayout;
		RenderPass::AttachmentLoadOp	loadOp;
		RenderPass::AttachmentStoreOp	storeOp;
		Image::ClearValue				clearValue;
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
