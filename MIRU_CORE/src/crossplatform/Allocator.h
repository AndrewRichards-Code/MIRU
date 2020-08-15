#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace crossplatform
{
	class Context;

	struct Resource
	{
		enum class Type : uint32_t
		{
			BUFFER,
			IMAGE
		};

		void*		device;
		Type		type;
		uint64_t	resource;
		uint32_t	usage;
		size_t		size;
		size_t		alignment;
		size_t		rowPitch;
		size_t		rowPadding;
		size_t		height;

		size_t		offset = 0;
		uint64_t	memoryBlock = 0;
		uint64_t	id = 0;
		bool		newMemoryBlock = false;
	};

	class MemoryBlock : public std::enable_shared_from_this<MemoryBlock>
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
		static Ref<MemoryBlock> Create(MemoryBlock::CreateInfo* pCreateInfo);
		virtual ~MemoryBlock() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual bool AddResource(Resource& resource) = 0;
		virtual void RemoveResource(uint64_t id) = 0;
		virtual void SubmitData(const crossplatform::Resource& resource, size_t size, void* data) = 0;
		virtual void AccessData(const crossplatform::Resource& resource, size_t size, void* data) = 0;

		static inline std::vector<Ref<MemoryBlock>>& GetMemoryBlocks() { return s_MemoryBlocks; }
		static inline std::map<Ref<MemoryBlock>, std::map<uint64_t, Resource>>& GetAllocatedResources() { return s_AllocatedResources; }

	protected:
		void CalculateOffsets();
		bool ResourceBackable(Resource& resource);
		static inline const uint64_t GenerateURID() { return (ruid_src++); }
		inline Ref<crossplatform::MemoryBlock> get_this_shared_ptr() { return shared_from_this(); }

		//Members
	protected:
		CreateInfo m_CI = {};
		
		static std::vector<Ref<MemoryBlock>> s_MemoryBlocks;
		static std::map<Ref<MemoryBlock>, std::map<uint64_t, Resource>> s_AllocatedResources;

	private:
		static uint64_t ruid_src;
	};
}
}