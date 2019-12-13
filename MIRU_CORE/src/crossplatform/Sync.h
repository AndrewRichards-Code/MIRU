#pragma once
#include "common.h"
#include "Buffer.h"
#include "Image.h"

namespace miru
{
namespace crossplatform
{
	//Host-Device Synchronisation
	class Fence
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			const char* debugName;
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
		//Returns false if Fence is signaled.
		virtual bool GetStatus() = 0;
		//Returns false if Fence is signaled.
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
			const char* debugName;
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
			const char* debugName;
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
			INDIRECT_COMMAND_READ_BIT = 0x00000001,
			INDEX_READ_BIT = 0x00000002,
			VERTEX_ATTRIBUTE_READ_BIT = 0x00000004,
			UNIFORM_READ_BIT = 0x00000008,
			INPUT_ATTACHMENT_READ_BIT = 0x00000010,
			SHADER_READ_BIT = 0x00000020,
			SHADER_WRITE_BIT = 0x00000040,
			COLOR_ATTACHMENT_READ_BIT = 0x00000080,
			COLOR_ATTACHMENT_WRITE_BIT = 0x00000100,
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
			Type						type;					//For UsageBit::MEMORY, UsageBit::BUFFER and UsageBit::IMAGE
			AccessBit					srcAccess;				//For UsageBit::MEMORY, UsageBit::BUFFER and UsageBit::IMAGE
			AccessBit					dstAccess;				//For UsageBit::MEMORY, UsageBit::BUFFER and UsageBit::IMAGE
			uint32_t					srcQueueFamilyIndex;	//For UsageBit::BUFFER and UsageBit::IMAGE
			uint32_t					dstQueueFamilyIndex;	//For UsageBit::BUFFER and UsageBit::IMAGE
			Ref<Buffer>					pBuffer;				//For UsageBit::BUFFER
			uint64_t					offset;					//For UsageBit::BUFFER
			uint64_t					size;					//For UsageBit::BUFFER
			Ref<Image>					pImage;					//For UsageBit::IMAGE
			Image::Layout				oldLayout;				//For UsageBit::IMAGE
			Image::Layout				newLayout;				//For UsageBit::IMAGE
			Image::SubresourceRange		subresoureRange;		//For UsageBit::IMAGE
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
	#define MIRU_QUEUE_FAMILY_IGNORED (~0U);

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
