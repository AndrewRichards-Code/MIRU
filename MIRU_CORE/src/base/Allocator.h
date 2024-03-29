#pragma once

#include "miru_core_common.h"

namespace miru
{
namespace base
{
	typedef void* NativeAllocation;
	typedef void* NativeAllocator;

	struct MIRU_API Allocation
	{
		NativeAllocation	nativeAllocation;
		size_t				rowPitch;
		size_t				rowCount;
		size_t				rowPadding;
		size_t				slicePitch;
		size_t				sliceCount;
	};

	class MIRU_API Allocator
	{
		//enums/structs
	public:
		enum class BlockSize : uint32_t
		{
			BLOCK_SIZE_1MB		= 1048576,
			BLOCK_SIZE_2MB		= 2		* BLOCK_SIZE_1MB,
			BLOCK_SIZE_4MB		= 4		* BLOCK_SIZE_1MB,
			BLOCK_SIZE_8MB		= 8		* BLOCK_SIZE_1MB,
			BLOCK_SIZE_16MB		= 16	* BLOCK_SIZE_1MB,
			BLOCK_SIZE_32MB		= 32	* BLOCK_SIZE_1MB,
			BLOCK_SIZE_64MB		= 64	* BLOCK_SIZE_1MB,
			BLOCK_SIZE_128MB	= 128	* BLOCK_SIZE_1MB,
			BLOCK_SIZE_256MB	= 256	* BLOCK_SIZE_1MB
		};
		enum class PropertiesBit : uint32_t
		{
			DEVICE_LOCAL_BIT		= 0x00000001,
			HOST_VISIBLE_BIT		= 0x00000002,
			HOST_COHERENT_BIT		= 0x00000004,
			HOST_CACHED_BIT			= 0x00000008,
			LAZILY_ALLOCATED_BIT	= 0x00000010,
			PROTECTED_BIT			= 0x00000020
		};
		struct CreateInfo
		{
			std::string		debugName;
			ContextRef		context;
			BlockSize		blockSize;
			PropertiesBit	properties;
		};

		//Methods
	public:
		static AllocatorRef Create(Allocator::CreateInfo* pCreateInfo);
		virtual ~Allocator() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual NativeAllocator GetNativeAllocator() = 0;

		virtual void SubmitData(const Allocation& allocation, size_t offset, size_t size, void* data) = 0;
		virtual void AccessData(const Allocation& allocation, size_t offset, size_t size, void* data) = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}