#pragma once
#include "miru_core_common.h"
#include "Allocator.h"
#include "PipelineHelper.h"

namespace miru
{
namespace crossplatform
{
	class Swapchain;

	class Image
	{
		//enums/structs
	public:
		enum class Type : uint32_t
		{
			TYPE_1D = 0,
			TYPE_2D = 1,
			TYPE_3D = 2,
			TYPE_CUBE = 3,
			TYPE_1D_ARRAY = 4,
			TYPE_2D_ARRAY = 5,
			TYPE_CUBE_ARRAY = 6,
		};
		enum class Format : uint32_t
		{
			UNKNOWN = 0,
			R4G4_UNORM_PACK8 = 1,
			R4G4B4A4_UNORM_PACK16 = 2,
			B4G4R4A4_UNORM_PACK16 = 3,
			R5G6B5_UNORM_PACK16 = 4,
			B5G6R5_UNORM_PACK16 = 5,
			R5G5B5A1_UNORM_PACK16 = 6,
			B5G5R5A1_UNORM_PACK16 = 7,
			A1R5G5B5_UNORM_PACK16 = 8,
			R8_UNORM = 9,
			R8_SNORM = 10,
			R8_USCALED = 11,
			R8_SSCALED = 12,
			R8_UINT = 13,
			R8_SINT = 14,
			R8_SRGB = 15,
			R8G8_UNORM = 16,
			R8G8_SNORM = 17,
			R8G8_USCALED = 18,
			R8G8_SSCALED = 19,
			R8G8_UINT = 20,
			R8G8_SINT = 21,
			R8G8_SRGB = 22,
			R8G8B8_UNORM = 23,
			R8G8B8_SNORM = 24,
			R8G8B8_USCALED = 25,
			R8G8B8_SSCALED = 26,
			R8G8B8_UINT = 27,
			R8G8B8_SINT = 28,
			R8G8B8_SRGB = 29,
			B8G8R8_UNORM = 30,
			B8G8R8_SNORM = 31,
			B8G8R8_USCALED = 32,
			B8G8R8_SSCALED = 33,
			B8G8R8_UINT = 34,
			B8G8R8_SINT = 35,
			B8G8R8_SRGB = 36,
			R8G8B8A8_UNORM = 37,
			R8G8B8A8_SNORM = 38,
			R8G8B8A8_USCALED = 39,
			R8G8B8A8_SSCALED = 40,
			R8G8B8A8_UINT = 41,
			R8G8B8A8_SINT = 42,
			R8G8B8A8_SRGB = 43,
			B8G8R8A8_UNORM = 44,
			B8G8R8A8_SNORM = 45,
			B8G8R8A8_USCALED = 46,
			B8G8R8A8_SSCALED = 47,
			B8G8R8A8_UINT = 48,
			B8G8R8A8_SINT = 49,
			B8G8R8A8_SRGB = 50,
			A8B8G8R8_UNORM_PACK32 = 51,
			A8B8G8R8_SNORM_PACK32 = 52,
			A8B8G8R8_USCALED_PACK32 = 53,
			A8B8G8R8_SSCALED_PACK32 = 54,
			A8B8G8R8_UINT_PACK32 = 55,
			A8B8G8R8_SINT_PACK32 = 56,
			A8B8G8R8_SRGB_PACK32 = 57,
			A2R10G10B10_UNORM_PACK32 = 58,
			A2R10G10B10_SNORM_PACK32 = 59,
			A2R10G10B10_USCALED_PACK32 = 60,
			A2R10G10B10_SSCALED_PACK32 = 61,
			A2R10G10B10_UINT_PACK32 = 62,
			A2R10G10B10_SINT_PACK32 = 63,
			A2B10G10R10_UNORM_PACK32 = 64,
			A2B10G10R10_SNORM_PACK32 = 65,
			A2B10G10R10_USCALED_PACK32 = 66,
			A2B10G10R10_SSCALED_PACK32 = 67,
			A2B10G10R10_UINT_PACK32 = 68,
			A2B10G10R10_SINT_PACK32 = 69,
			R16_UNORM = 70,
			R16_SNORM = 71,
			R16_USCALED = 72,
			R16_SSCALED = 73,
			R16_UINT = 74,
			R16_SINT = 75,
			R16_SFLOAT = 76,
			R16G16_UNORM = 77,
			R16G16_SNORM = 78,
			R16G16_USCALED = 79,
			R16G16_SSCALED = 80,
			R16G16_UINT = 81,
			R16G16_SINT = 82,
			R16G16_SFLOAT = 83,
			R16G16B16_UNORM = 84,
			R16G16B16_SNORM = 85,
			R16G16B16_USCALED = 86,
			R16G16B16_SSCALED = 87,
			R16G16B16_UINT = 88,
			R16G16B16_SINT = 89,
			R16G16B16_SFLOAT = 90,
			R16G16B16A16_UNORM = 91,
			R16G16B16A16_SNORM = 92,
			R16G16B16A16_USCALED = 93,
			R16G16B16A16_SSCALED = 94,
			R16G16B16A16_UINT = 95,
			R16G16B16A16_SINT = 96,
			R16G16B16A16_SFLOAT = 97,
			R32_UINT = 98,
			R32_SINT = 99,
			R32_SFLOAT = 100,
			R32G32_UINT = 101,
			R32G32_SINT = 102,
			R32G32_SFLOAT = 103,
			R32G32B32_UINT = 104,
			R32G32B32_SINT = 105,
			R32G32B32_SFLOAT = 106,
			R32G32B32A32_UINT = 107,
			R32G32B32A32_SINT = 108,
			R32G32B32A32_SFLOAT = 109,
			R64_UINT = 110,
			R64_SINT = 111,
			R64_SFLOAT = 112,
			R64G64_UINT = 113,
			R64G64_SINT = 114,
			R64G64_SFLOAT = 115,
			R64G64B64_UINT = 116,
			R64G64B64_SINT = 117,
			R64G64B64_SFLOAT = 118,
			R64G64B64A64_UINT = 119,
			R64G64B64A64_SINT = 120,
			R64G64B64A64_SFLOAT = 121,
			B10G11R11_UFLOAT_PACK32 = 122,
			E5B9G9R9_UFLOAT_PACK32 = 123,
			D16_UNORM = 124,
			X8_D24_UNORM_PACK32 = 125,
			D32_SFLOAT = 126,
			S8_UINT = 127,
			D16_UNORM_S8_UINT = 128,
			D24_UNORM_S8_UINT = 129,
			D32_SFLOAT_S8_UINT = 130,
		};
		enum class SampleCountBit : uint32_t
		{
			SAMPLE_COUNT_1_BIT	= 0x00000001,
			SAMPLE_COUNT_2_BIT	= 0x00000002,
			SAMPLE_COUNT_4_BIT	= 0x00000004,
			SAMPLE_COUNT_8_BIT	= 0x00000008,
			SAMPLE_COUNT_16_BIT	= 0x00000010,
			SAMPLE_COUNT_32_BIT	= 0x00000020,
			SAMPLE_COUNT_64_BIT	= 0x00000040,
		};
		enum class UsageBit : uint32_t
		{
			TRANSFER_SRC_BIT				= 0x00000001,
			TRANSFER_DST_BIT				= 0x00000002,
			SAMPLED_BIT						= 0x00000004,
			STORAGE_BIT						= 0x00000008,
			COLOUR_ATTACHMENT_BIT			= 0x00000010,
			DEPTH_STENCIL_ATTACHMENT_BIT	= 0x00000020,
			TRANSIENT_ATTACHMENT_BIT		= 0x00000040,
			INPUT_ATTACHMENT_BIT			= 0x00000080
		};
		enum class Layout : uint32_t
		{
			UNKNOWN = 0,
			GENERAL = 1,
			COLOUR_ATTACHMENT_OPTIMAL = 2,
			DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
			DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
			SHADER_READ_ONLY_OPTIMAL = 5,
			TRANSFER_SRC_OPTIMAL = 6,
			TRANSFER_DST_OPTIMAL = 7,
			PREINITIALIZED = 8,
			DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
			DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
			PRESENT_SRC = 1000001002,
			SHARED_PRESENT = 1000111000,
		};
		enum class AspectBit : uint32_t
		{
			COLOUR_BIT		= 0x00000001,
			DEPTH_BIT		= 0x00000002,
			STENCIL_BIT		= 0x00000004,
			METADATA_BIT	= 0x00000008,
			PLANE_0_BIT		= 0x00000010,
			PLANE_1_BIT		= 0x00000020,
			PLANE_2_BIT		= 0x00000040,
		};
		struct SubresourceRange
		{
			AspectBit	aspect;
			uint32_t	baseMipLevel;
			uint32_t	mipLevelCount;
			uint32_t	baseArrayLayer;
			uint32_t	arrayLayerCount;
		};
		struct SubresourceLayers
		{
			AspectBit	aspectMask;
			uint32_t	mipLevel;
			uint32_t	baseArrayLayer;
			uint32_t	arrayLayerCount;
		};
		struct Copy
		{
			SubresourceLayers	srcSubresource;
			Offset3D			srcOffset;
			SubresourceLayers	dstSubresource;
			Offset3D			dstOffset;
			Extent3D			extent;
		};
		struct BufferImageCopy 
		{
			uint64_t			bufferOffset;
			uint32_t			bufferRowLength;
			uint32_t			bufferImageHeight;
			SubresourceLayers	imageSubresource;
			Offset3D			imageOffset;
			Extent3D			imageExtent;
		};
		
		union ClearColourValue 
		{
			float       float32[4];
			int32_t     int32[4];
			uint32_t    uint32[4];
		};
		struct ClearDepthStencilValue 
		{
			float       depth;
			uint32_t    stencil;
		};
		union ClearValue 
		{
			ClearColourValue          colour;
			ClearDepthStencilValue    depthStencil;
		};

		struct CreateInfo
		{
			std::string			debugName;
			void*				device;
			Type				type;
			Format				format;
			uint32_t			width;
			uint32_t			height;
			uint32_t			depth;
			uint32_t			mipLevels;
			uint32_t			arrayLayers;
			SampleCountBit		sampleCount;
			UsageBit			usage;
			Layout				layout;
			size_t				size;
			void*				data;
			Ref<MemoryBlock>	pMemoryBlock;
		};

		//Methods
	public:
		static Ref<Image> Create(CreateInfo* pCreateInfo);
		virtual ~Image() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		const Resource& GetResource() { return m_Resource; }
		friend Swapchain;

		//Members
	protected:
		CreateInfo m_CI = {};
		Resource m_Resource;
		bool m_SwapchainImage = false;
	};

	class ImageView
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			std::string				debugName;
			void*					device;
			Ref<Image>				pImage;
			Image::Type				viewType;
			Image::SubresourceRange subresourceRange;
		};

		//Methods
	public:
		static Ref<ImageView> Create(CreateInfo* pCreateInfo);
		virtual ~ImageView() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		friend Swapchain;

		//Members
	protected:
		CreateInfo m_CI = {};
		bool m_SwapchainImageView = false;
	};

	class Sampler
	{
		//enums/structs
	public:
		enum class Filter : uint32_t
		{
			NEAREST,
			LINEAR
		};
		enum class MipmapMode : uint32_t
		{
			NEAREST,
			LINEAR
		};
		enum class AddressMode : uint32_t
		{
			REPEAT = 0,
			MIRRORED_REPEAT = 1,
			CLAMP_TO_EDGE = 2,
			CLAMP_TO_BORDER = 3,
			MIRROR_CLAMP_TO_EDGE = 4
		};
		enum class BorderColour : uint32_t
		{
			FLOAT_TRANSPARENT_BLACK = 0,
			INT_TRANSPARENT_BLACK = 1,
			FLOAT_OPAQUE_BLACK = 2,
			INT_OPAQUE_BLACK = 3,
			FLOAT_OPAQUE_WHITE = 4,
			INT_OPAQUE_WHITE = 5,
		};
		struct CreateInfo
		{
			std::string		debugName;
			void*			device;
			Filter			magFilter;
			Filter			minFilter;
			MipmapMode		mipmapMode;
			AddressMode		addressModeU;
			AddressMode		addressModeV;
			AddressMode		addressModeW;
			float			mipLodBias;
			bool			anisotropyEnable;
			float			maxAnisotropy;
			bool			compareEnable;
			CompareOp		compareOp;
			float			minLod;
			float			maxLod;
			BorderColour	borderColour;
			bool			unnormalisedCoordinates;
		};

		//Methods
	public:
		static Ref<Sampler> Create(CreateInfo* pCreateInfo);
		virtual ~Sampler() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}