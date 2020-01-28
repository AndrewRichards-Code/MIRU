#pragma once
#include "common.h"
#include "Shader.h"
#include "Image.h"
#include "DescriptorPoolSet.h"
#include "Sync.h"

namespace miru
{
namespace crossplatform
{
	//VertexInputState

	enum class VertexInputRate : uint32_t
	{
		VERTEX,
		INSTANCE
	};
	struct VertexInputBindingDescription
	{
		uint32_t			binding;
		uint32_t			stride;
		VertexInputRate		inputRate;
	};
	enum class VertexType : uint32_t
	{
		FLOAT,
		DOUBLE,
		VEC2,
		VEC3,
		VEC4,
		IVEC2,
		IVEC3,
		IVEC4,
		UVEC2,
		UVEC3,
		UVEC4
	};
	struct VertexInputAttributeDescription
	{
		uint32_t	location;
		uint32_t	binding;
		VertexType	vertexType;
		uint32_t	offset;
	};
	
	//InputAssemblyState

	enum class PrimitiveTopology : uint32_t
	{
		POINT_LIST = 0,
		LINE_LIST = 1,
		LINE_STRIP = 2,
		TRIANGLE_LIST = 3,
		TRIANGLE_STRIP = 4,
		TRIANGLE_FAN = 5,
		LINE_LIST_WITH_ADJACENCY = 6,
		LINE_STRIP_WITH_ADJACENCY = 7,
		TRIANGLE_LIST_WITH_ADJACENCY = 8,
		TRIANGLE_STRIP_WITH_ADJACENCY = 9,
		PATCH_LIST = 10,
	};
	
	//ViewportState
	struct Viewport
	{
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};
	struct Offset2D
	{
		int32_t x;
		int32_t y;
	};
	struct Extent2D
	{
		uint32_t width;
		uint32_t height;
	};
	struct Rect2D
	{
		Offset2D offset;
		Extent2D extent;
	};
	
	//RasterisationState

	enum class PolygonMode : uint32_t
	{
		FILL = 0,
		LINE = 1,
		POINT = 2,
	};
	enum class CullModeBit : uint32_t
	{
		NONE = 0x00000000,
		FRONT_BIT = 0x00000001,
		BACK_BIT = 0x00000002,
		FRONT_AND_BACK = 0x00000003,
	};
	enum class FrontFace {
		COUNTER_CLOCKWISE = 0,
		CLOCKWISE = 1,
	};
	
	//DepthStencilState

	enum StencilOp : uint32_t
	{
		KEEP = 0,
		ZERO = 1,
		REPLACE = 2,
		INCREMENT_AND_CLAMP = 3,
		DECREMENT_AND_CLAMP = 4,
		INVERT = 5,
		INCREMENT_AND_WRAP = 6,
		DECREMENT_AND_WRAP = 7
	};
	enum class CompareOp : uint32_t
	{
		NEVER = 0,
		LESS = 1,
		EQUAL = 2,
		LESS_OR_EQUAL = 3,
		GREATER = 4,
		NOT_EQUAL = 5,
		GREATER_OR_EQUAL = 6,
		ALWAYS = 7,
	};
	struct StencilOpState
	{
		StencilOp	failOp;
		StencilOp	passOp;
		StencilOp	depthFailOp;
		CompareOp	compareOp;
		uint32_t	compareMask;
		uint32_t	writeMask;
		uint32_t	reference;
	};
	
	//ColourBlendState

	enum class BlendFactor : uint32_t
	{
		ZERO = 0,
		ONE = 1,
		SRC_COLOR = 2,
		ONE_MINUS_SRC_COLOR = 3,
		DST_COLOR = 4,
		ONE_MINUS_DST_COLOR = 5,
		SRC_ALPHA = 6,
		ONE_MINUS_SRC_ALPHA = 7,
		DST_ALPHA = 8,
		ONE_MINUS_DST_ALPHA = 9,
		CONSTANT_COLOR = 10,
		ONE_MINUS_CONSTANT_COLOR = 11,
		CONSTANT_ALPHA = 12,
		ONE_MINUS_CONSTANT_ALPHA = 13,
		SRC_ALPHA_SATURATE = 14,
		SRC1_COLOR = 15,
		ONE_MINUS_SRC1_COLOR = 16,
		SRC1_ALPHA = 17,
		ONE_MINUS_SRC1_ALPHA = 18
	};
	enum class BlendOp : uint32_t
	{
		ADD = 0,
		SUBTRACT = 1,
		REVERSE_SUBTRACT = 2,
		MIN = 3,
		MAX = 4,
	};
	enum class ColourComponentBit : uint32_t 
	{
		R_BIT = 0x00000001,
		G_BIT = 0x00000002,
		B_BIT = 0x00000004,
		A_BIT = 0x00000008,
	};
	struct ColourBlendAttachmentState
	{
		bool					blendEnable;
		BlendFactor				srcColourBlendFactor;
		BlendFactor				dstColourBlendFactor;
		BlendOp					colorBlendOp;
		BlendFactor				srcAlphaBlendFactor;
		BlendFactor				dstAlphaBlendFactor;
		BlendOp					alphaBlendOp;
		ColourComponentBit		colourWriteMask;
	};
	
	enum class LogicOp : uint32_t
	{
		CLEAR = 0,
		AND = 1,
		AND_REVERSE = 2,
		COPY = 3,
		AND_INVERTED = 4,
		NO_OP = 5,
		XOR = 6,
		OR = 7,
		NOR = 8,
		EQUIVALENT = 9,
		INVERT = 10,
		OR_REVERSE = 11,
		COPY_INVERTED = 12,
		OR_INVERTED = 13,
		NAND = 14,
		SET = 15
	};
	
	//DynamicStates

	enum class DynamicState : uint32_t
	{
		VIEWPORT = 0,
		SCISSOR = 1,
		LINE_WIDTH = 2,
		DEPTH_BIAS = 3,
		BLEND_CONSTANTS = 4,
		DEPTH_BOUNDS = 5,
		STENCIL_COMPARE_MASK = 6,
		STENCIL_WRITE_MASK = 7,
		STENCIL_REFERENCE = 8,
	};

	//PushConstantRange

	struct PushConstantRange
	{
		Shader::StageBit	stages;
		uint32_t			offset;
		uint32_t			size;
	};

	//Other

	enum class PipelineType : uint32_t
	{
		GRAPHICS,
		COMPUTE,
		RAY_TRACING_NV = 1000165000
	};
	enum class PipelineStageBit : uint32_t
	{
		NONE = 0x00000000,
		TOP_OF_PIPE_BIT = 0x00000001,
		DRAW_INDIRECT_BIT = 0x00000002,
		VERTEX_INPUT_BIT = 0x00000004,
		VERTEX_SHADER_BIT = 0x00000008,
		TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
		TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
		GEOMETRY_SHADER_BIT = 0x00000040,
		FRAGMENT_SHADER_BIT = 0x00000080,
		EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
		LATE_FRAGMENT_TESTS_BIT = 0x00000200,
		COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
		COMPUTE_SHADER_BIT = 0x00000800,
		TRANSFER_BIT = 0x00001000,
		BOTTOM_OF_PIPE_BIT = 0x00002000,

		HOST_BIT = 0x00004000,
		ALL_GRAPHICS_BIT = 0x00008000,
		ALL_COMMANDS_BIT = 0x00010000,
		COMMAND_PROCESS_BIT_NVX = 0x00020000,
		SHADING_RATE_IMAGE_BIT_NV = 0x00400000,
		RAY_TRACING_SHADER_BIT_NV = 0x00200000,
		ACCELERATION_STRUCTURE_BUILD_BIT_NV = 0x02000000,
		TASK_SHADER_BIT_NV = 0x00080000,
		MESH_SHADER_BIT_NV = 0x00100000,

		HULL_SHADER_BIT = TESSELLATION_CONTROL_SHADER_BIT,
		DOMAIN_SHADER_BIT = TESSELLATION_EVALUATION_SHADER_BIT,
		PIXEL_SHADER_BIT = FRAGMENT_SHADER_BIT,
	};
}
}