#pragma once
#include "miru_core_common.h"
#include "Allocator.h"

namespace miru
{
namespace base
{
	class MIRU_API Buffer
	{
		//enums/structs
	public:
		struct ImageDimension
		{
			uint32_t width;
			uint32_t height;
			uint32_t pixelSize;
		};
		struct Copy
		{
			size_t srcOffset;
			size_t dstOffset;
			size_t size;
		};
		enum class UsageBit : uint32_t
		{
			TRANSFER_SRC_BIT									= 0x00000001,
			TRANSFER_DST_BIT									= 0x00000002,
			UNIFORM_TEXEL_BIT									= 0x00000004,
			STORAGE_TEXEL_BIT									= 0x00000008,
			UNIFORM_BIT											= 0x00000010,
			STORAGE_BIT											= 0x00000020,
			INDEX_BIT											= 0x00000040,
			VERTEX_BIT											= 0x00000080,
			INDIRECT_BIT										= 0x00000100,
			CONDITIONAL_RENDERING_BIT							= 0x00000200,
			SHADER_BINDING_TABLE_BIT							= 0x00000400,
			TRANSFORM_FEEDBACK_BIT								= 0x00000800,
			TRANSFORM_FEEDBACK_COUNTER_BIT						= 0x00001000,
			SHADER_DEVICE_ADDRESS_BIT							= 0x00020000,
			ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT	= 0x00080000,
			ACCELERATION_STRUCTURE_STORAGE_BIT					= 0x00100000,
		};
		struct CreateInfo
		{
			std::string		debugName;
			void*			device;
			UsageBit		usage;
			ImageDimension	imageDimension = { 0, 0, 0 }; //For D3D12 only: If this buffer is an upload for an image.
			size_t			size;
			void*			data;
			Ref<Allocator>	pAllocator;
		};

		//Methods
	public:
		static Ref<Buffer> Create(CreateInfo* pCreateInfo);
		virtual ~Buffer() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		const Allocation& GetAllocation() { return m_Allocation; }

		//Members
	protected:
		CreateInfo m_CI = {};
		Allocation m_Allocation;
	};
	MIRU_CLASS_REF_TYPEDEF(Buffer);
	
	class MIRU_API BufferView
	{
		//enums/structs
	public:
		enum class Type : uint32_t
		{
			UNIFORM_TEXEL,
			STORAGE_TEXEL,
			UNIFORM,
			STORAGE,
			INDEX,
			VERTEX,
			TRANSFORM_FEEDBACK
		};
		struct CreateInfo
		{
			std::string	debugName;
			void*		device;
			Type		type;
			Ref<Buffer>	pBuffer;
			size_t		offset;
			size_t		size;
			size_t		stride;
		};

		//Methods
	public:
		static Ref<BufferView> Create(CreateInfo* pCreateInfo);
		virtual ~BufferView() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
	MIRU_CLASS_REF_TYPEDEF(BufferView);
}
}