#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace crossplatform
{
	typedef void* NativeAllocation;
	typedef void* NativeAllocator;
	
	class Context;

	struct MIRU_API Allocation
	{
		NativeAllocation	nativeAllocation;
		size_t				width;
		size_t				height;
		size_t				rowPitch;
		size_t				rowPadding;

		#if defined(MIRU_D3D12)
		inline const D3D12MA::Allocation* GetD3D12MAAllocaton() const  { return reinterpret_cast<D3D12MA::Allocation*>(nativeAllocation); }
		#endif
		#if defined(MIRU_VULKAN)
		inline const VmaAllocation& GetVmaAllocation() const { return *reinterpret_cast<VmaAllocation*>(nativeAllocation); }
		#endif
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
			Ref<Context>	pContext;
			BlockSize		blockSize;
			PropertiesBit	properties;
		};

		//Methods
	public:
		static Ref<Allocator> Create(Allocator::CreateInfo* pCreateInfo);
		virtual ~Allocator() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual NativeAllocator GetNativeAllocator() = 0;

		virtual void SubmitData(const Allocation& allocation, size_t size, void* data) = 0;
		virtual void AccessData(const Allocation& allocation, size_t size, void* data) = 0;

		#if defined(MIRU_D3D12)
		inline D3D12MA::Allocator* GetD3D12MAAllocator() { return reinterpret_cast<D3D12MA::Allocator*>(GetNativeAllocator()); }
		#endif
		#if defined(MIRU_VULKAN)
		inline VmaAllocator GetVmaAllocator() { return *reinterpret_cast<VmaAllocator*>(GetNativeAllocator()); };
		#endif

		//Members
	protected:
		CreateInfo m_CI = {};
	};
	MIRU_CLASS_REF_TYPEDEF(Allocator);
}
}