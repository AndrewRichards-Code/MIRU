#pragma once

#include "miru_core_common.h"
#include "Image.h"

namespace miru
{
namespace base
{
	class Buffer;

	//Host-Device Synchronisation
	class MIRU_API Fence
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			bool		signaled;
			uint64_t	timeout;	//In nanoseconds
		};
		//Methods
	public:
		static FenceRef Create(Fence::CreateInfo* pCreateInfo);
		virtual ~Fence() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Reset() = 0;
		//Returns true if Fence is signaled
		virtual bool GetStatus() = 0;
		//Returns false if the wait times out
		virtual bool Wait() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Queue Synchronisation
	class MIRU_API Semaphore
	{
		//enum/struct
	public:
		enum class Type : uint32_t
		{
			BINARY,
			TIMELINE
		};
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			Type		type;
		};
		//Methods
	public:
		static SemaphoreRef Create(Semaphore::CreateInfo* pCreateInfo);
		virtual ~Semaphore() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Signal(uint64_t value) = 0;
		//Parameter: timeout is in nanoseconds
		virtual bool Wait(uint64_t value, uint64_t timeout) = 0;
		virtual uint64_t GetCurrentValue() = 0;
		
		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Command Buffer Synchronisation
	class MIRU_API Event
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
		};
		//Methods
	public:
		static EventRef Create(Event::CreateInfo* pCreateInfo);
		virtual ~Event() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Set() = 0;
		virtual void Reset() = 0;
		//Returns false if Event is signaled
		virtual bool GetStatus() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Command Buffer Resource Synchronisation
	class MIRU_API Barrier
	{
		//enum/struct
	public:
		enum class Type : uint32_t
		{
			MEMORY,
			BUFFER,
			IMAGE
		};
		enum class AccessBit : uint64_t
		{
			NONE_BIT									= 0x00000000,
			INDIRECT_COMMAND_READ_BIT					= 0x00000001,
			INDEX_READ_BIT								= 0x00000002,
			VERTEX_ATTRIBUTE_READ_BIT					= 0x00000004,
			UNIFORM_READ_BIT							= 0x00000008,
			INPUT_ATTACHMENT_READ_BIT					= 0x00000010,
			SHADER_READ_BIT								= 0x00000020,
			SHADER_WRITE_BIT							= 0x00000040,
			COLOUR_ATTACHMENT_READ_BIT					= 0x00000080,
			COLOUR_ATTACHMENT_WRITE_BIT					= 0x00000100,
			DEPTH_STENCIL_ATTACHMENT_READ_BIT			= 0x00000200,
			DEPTH_STENCIL_ATTACHMENT_WRITE_BIT			= 0x00000400,
			TRANSFER_READ_BIT							= 0x00000800,
			TRANSFER_WRITE_BIT							= 0x00001000,
			HOST_READ_BIT								= 0x00002000,
			HOST_WRITE_BIT								= 0x00004000,
			MEMORY_READ_BIT								= 0x00008000,
			MEMORY_WRITE_BIT							= 0x00010000,

			//Extensions

			TRANSFORM_FEEDBACK_WRITE_BIT				= 0x02000000,
			TRANSFORM_FEEDBACK_COUNTER_READ_BIT			= 0x04000000,
			TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT		= 0x08000000,
			CONDITIONAL_RENDERING_READ_BIT				= 0x00100000,
			FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT	= 0x00800000,
			ACCELERATION_STRUCTURE_READ_BIT				= 0x00200000,
			ACCELERATION_STRUCTURE_WRITE_BIT			= 0x00400000,
			FRAGMENT_DENSITY_MAP_READ_BIT				= 0x01000000,
			VIDEO_DECODE_READ_BIT						= 0x0000000800000000,
			VIDEO_DECODE_WRITE_BIT						= 0x0000001000000000,
			VIDEO_ENCODE_READ_BIT						= 0x0000002000000000,
			VIDEO_ENCODE_WRITE_BIT						= 0x0000004000000000,
			SHADER_BINDING_TABLE_READ_BIT				= 0x0000010000000000,

			//SYNCHRONISATION_2

			SHADER_SAMPLED_READ_BIT						= 0x0000000100000000,
			SHADER_STORAGE_READ_BIT						= 0x0000000200000000,
			SHADER_STORAGE_WRITE_BIT					= 0x0000000400000000,

			D3D12_RESOLVE_SOURCE						= 0x0000100000000000,
			D3D12_RESOLVE_DEST							= 0x0000200000000000,
		};
		struct CreateInfo
		{
			Type						type;					//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			AccessBit					srcAccess;				//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			AccessBit					dstAccess;				//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			uint32_t					srcQueueFamilyIndex;	//For Type::BUFFER and Type::IMAGE
			uint32_t					dstQueueFamilyIndex;	//For Type::BUFFER and Type::IMAGE
			BufferRef					buffer;					//For Type::BUFFER
			uint64_t					offset;					//For Type::BUFFER
			uint64_t					size;					//For Type::BUFFER
			ImageRef					image;					//For Type::IMAGE
			Image::Layout				oldLayout;				//For Type::IMAGE
			Image::Layout				newLayout;				//For Type::IMAGE
			Image::SubresourceRange		subresourceRange;		//For Type::IMAGE
		};
		//Methods
	public:
		static BarrierRef Create(Barrier::CreateInfo* pCreateInfo);
		virtual ~Barrier() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		inline const CreateInfo& GetCreateInfo() const { return m_CI; }
		
		static constexpr uint32_t QueueFamilyIgnored = ~0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Command Buffer Resource Synchronisation
	class MIRU_API Barrier2
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			Barrier::Type				type;					//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			PipelineStageBit			srcStageMask;			//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			Barrier::AccessBit			srcAccess;				//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			PipelineStageBit			dstStageMask;			//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			Barrier::AccessBit			dstAccess;				//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			uint32_t					srcQueueFamilyIndex;	//For Type::BUFFER and Type::IMAGE
			uint32_t					dstQueueFamilyIndex;	//For Type::BUFFER and Type::IMAGE
			BufferRef					buffer;					//For Type::BUFFER
			uint64_t					offset;					//For Type::BUFFER
			uint64_t					size;					//For Type::BUFFER
			ImageRef					image;					//For Type::IMAGE
			Image::Layout				oldLayout;				//For Type::IMAGE
			Image::Layout				newLayout;				//For Type::IMAGE
			Image::SubresourceRange		subresourceRange;		//For Type::IMAGE
		};
		//Methods
	public:
		static Barrier2Ref Create(Barrier2::CreateInfo* pCreateInfo);
		virtual ~Barrier2() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		inline const CreateInfo& GetCreateInfo() const { return m_CI; }

		static constexpr uint32_t QueueFamilyIgnored = ~0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}
