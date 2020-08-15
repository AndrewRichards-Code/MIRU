#pragma once
#include "miru_core_common.h"
#include "Image.h"

namespace miru
{
namespace crossplatform
{
	class Buffer;

	//Host-Device Synchronisation
	class Fence
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			bool		signaled;
			uint64_t	timeout; //In nanoseconds
		};
		//Methods
	public:
		static Ref<Fence> Create(Fence::CreateInfo* pCreateInfo);
		virtual ~Fence() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Reset() = 0;
		//Returns true if Fence is signaled.
		virtual bool GetStatus() = 0;
		//Returns false if the wait times out.
		virtual bool Wait() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Queue Synchronisation
	class Semaphore
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
		static Ref<Semaphore> Create(Semaphore::CreateInfo* pCreateInfo);
		virtual ~Semaphore() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Command Buffer Synchronisation
	class Event
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
		static Ref<Event> Create(Event::CreateInfo* pCreateInfo);
		virtual ~Event() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Set() = 0;
		virtual void Reset() = 0;
		//Returns false if Event is signaled.
		virtual bool GetStatus() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	//Inter/Intra-Command Buffer Resource Synchronisation
	class Barrier
	{
		//enum/struct
	public:
		enum class Type : uint32_t
		{
			MEMORY,
			BUFFER,
			IMAGE
		};
		enum class AccessBit : uint32_t
		{
			NONE = 0x00000000,
			INDIRECT_COMMAND_READ_BIT = 0x00000001,
			INDEX_READ_BIT = 0x00000002,
			VERTEX_ATTRIBUTE_READ_BIT = 0x00000004,
			UNIFORM_READ_BIT = 0x00000008,
			INPUT_ATTACHMENT_READ_BIT = 0x00000010,
			SHADER_READ_BIT = 0x00000020,
			SHADER_WRITE_BIT = 0x00000040,
			COLOUR_ATTACHMENT_READ_BIT = 0x00000080,
			COLOUR_ATTACHMENT_WRITE_BIT = 0x00000100,
			DEPTH_STENCIL_ATTACHMENT_READ_BIT = 0x00000200,
			DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 0x00000400,
			TRANSFER_READ_BIT = 0x00000800,
			TRANSFER_WRITE_BIT = 0x00001000,
			HOST_READ_BIT = 0x00002000,
			HOST_WRITE_BIT = 0x00004000,
			MEMORY_READ_BIT = 0x00008000,
			MEMORY_WRITE_BIT = 0x00010000,
			TRANSFORM_FEEDBACK_WRITE_BIT_EXT = 0x02000000,
			TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT = 0x04000000,
			TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT = 0x08000000,
		};
		struct CreateInfo
		{
			Type						type;					//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			AccessBit					srcAccess;				//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			AccessBit					dstAccess;				//For Type::MEMORY, Type::BUFFER and Type::IMAGE
			uint32_t					srcQueueFamilyIndex;	//For Type::BUFFER and Type::IMAGE
			uint32_t					dstQueueFamilyIndex;	//For Type::BUFFER and Type::IMAGE
			Ref<Buffer>					pBuffer;				//For Type::BUFFER. For D3D12 UAV Barriers on buffers.
			uint64_t					offset;					//For Type::BUFFER
			uint64_t					size;					//For Type::BUFFER
			Ref<Image>					pImage;					//For Type::IMAGE For D3D12 UAV Barriers on images.
			Image::Layout				oldLayout;				//For Type::IMAGE
			Image::Layout				newLayout;				//For Type::IMAGE
			Image::SubresourceRange		subresoureRange;		//For Type::IMAGE
		};
		//Methods
	public:
		static Ref<Barrier> Create(Barrier::CreateInfo* pCreateInfo);
		virtual ~Barrier() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		inline const CreateInfo& GetCreateInfo() const { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
	#define MIRU_QUEUE_FAMILY_IGNORED (~0U)
}
}
