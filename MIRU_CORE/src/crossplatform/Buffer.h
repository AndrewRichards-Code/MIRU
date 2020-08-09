#pragma once
#include "miru_core_common.h"
#include "Allocator.h"

namespace miru
{
namespace crossplatform
{
	class Buffer
	{
		//enums/structs
	public:
		struct ImageDimension
		{
			uint32_t width;
			uint32_t height;
			uint32_t channels;
		};
		struct Copy
		{
			size_t srcOffset;
			size_t dstOffset;
			size_t size;
		};
		enum class UsageBit : uint32_t
		{
			TRANSFER_SRC		= 0x00000001,
			TRANSFER_DST		= 0x00000002,
			UNIFORM_TEXEL		= 0x00000004,
			STORAGE_TEXEL		= 0x00000008,
			UNIFORM				= 0x00000010,
			STORAGE				= 0x00000020,
			INDEX				= 0x00000040,
			VERTEX				= 0x00000080,
			INDIRECT			= 0x00000100,
			TRANSFORM_FEEDBACK	= 0x00000800
		};
		struct CreateInfo
		{
			const char*			debugName;
			void*				device;
			UsageBit			usage;
			ImageDimension		imageDimension = { 0, 0, 0 }; //For D3D12 only: If this buffer is an upload for a image.
			size_t				size;
			void*				data;
			Ref<MemoryBlock>	pMemoryBlock;
		};

		//Methods
	public:
		static Ref<Buffer> Create(CreateInfo* pCreateInfo);
		virtual ~Buffer() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		const Resource& GetResource() { return m_Resource; }

		//Members
	protected:
		CreateInfo m_CI = {};
		Resource m_Resource;
	};

	class BufferView
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
			const char*			debugName;
			void*				device;
			Type				type;
			Ref<Buffer>			pBuffer;
			size_t				offset;
			size_t				size;
			size_t				stride;
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
}
}