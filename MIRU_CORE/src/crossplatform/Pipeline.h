#pragma once
#include "common.h"
#include "PipelineHelper.h"

namespace miru
{
namespace crossplatform
{
	class RenderPass
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
			DONT_CARE
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
			uint32_t srcSubpass;			//Index of the Subpass that this subpass is dependent on.
			uint32_t dstSubpass;			//Index of this subpass.
			PipelineStageBit srcStage;	//Which stage of the previous subpass needs to by completed?
			PipelineStageBit dstStage;	//Which stage of this subpass needs to by completed?
			Barrier::AccessBit srcAccess;	//What access is needed into the previous subpass stage?
			Barrier::AccessBit dstAccess;	//What access is needed into this subpass stage?
		};
		#define MIRU_SUBPASS_EXTERNAL (~0U)
		struct CreateInfo
		{
			const char*							debugName;
			void*								device;
			std::vector<AttachmentDescription>	attachments;				//List of a images and their usage throughout the whole RenderPass.
			std::vector<SubpassDescription>		subpassDescriptions;		//List of subpass to executed with the RenderPass.
			std::vector<SubpassDependency>		subpassDependencies;		//List of dependency between subpass. One subpass may have multiple dependcies on other subpasses.
		};

		//Methods
	public:
		static Ref<RenderPass> Create(CreateInfo* pCreateInfo);
		virtual ~RenderPass() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};

	};
	class Pipeline
	{
		//enums/structs
	public:

		struct VertexInputState
		{
			std::vector<VertexInputBindingDescription> vertexInputBindingDescriptions;
			std::vector<VertexInputAttributeDescription> vertexInputAttributeDescriptions;
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
			std::vector<Ref<DescriptorSetLayout>> descriptorSetLayouts;
			std::vector<PushConstantRange>		  pushConstantRanges;
		};
		struct CreateInfo
		{
			const char*					debugName;
			void*						device;
			PipelineType				type;
			std::vector<Ref<Shader>>	shaders;			//One shader only for compute.
			VertexInputState			vertexInputState;	//Graphics only.
			InputAssemblyState			inputAssemblyState;	//Graphics only.
			TessellationState			tessellationState;	//Graphics only.
			ViewportState				viewportState;		//Graphics only.
			RasterisationState			rasterisationState;	//Graphics only.
			MultisampleState			multisampleState;	//Graphics only.
			DepthStencilState			depthStencilState;	//Graphics only.
			ColourBlendState			colourBlendState;	//Graphics only.
			DynamicStates				dynamicStates;		//Graphics only.
			PipelineLayout				layout;				//Both.
			Ref<RenderPass>				renderPass;			//Graphics only.
			uint32_t					subpassIndex;		//Graphics only.
		};

		//Methods
	public:
		static Ref<Pipeline> Create(CreateInfo* pCreateInfo);
		virtual ~Pipeline() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}
