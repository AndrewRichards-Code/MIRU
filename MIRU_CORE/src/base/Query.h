#pragma once
#include "miru_core_common.h"
#include <filesystem>

namespace miru
{
	namespace base
	{
		class MIRU_API QueryPool
		{
			//enums/structs
		public:
			enum class Type : uint32_t
			{
				OCCLUSION = 0,
				PIPELINE_STATISTICS = 1,
				TIMESTAMP = 2
			};

			enum class PipelineStatisticFlagBit : uint32_t
			{
				NONE_BIT = 0x00000000,
				INPUT_ASSEMBLY_VERTICES_BIT = 0x00000001,
				INPUT_ASSEMBLY_PRIMITIVES_BIT = 0x00000002,
				VERTEX_SHADER_INVOCATIONS_BIT = 0x00000004,
				GEOMETRY_SHADER_INVOCATIONS_BIT = 0x00000008,
				GEOMETRY_SHADER_PRIMITIVES_BIT = 0x00000010,
				CLIPPING_INVOCATIONS_BIT = 0x00000020,
				CLIPPING_PRIMITIVES_BIT = 0x00000040,
				FRAGMENT_SHADER_INVOCATIONS_BIT = 0x00000080,
				TESSELLATION_CONTROL_SHADER_PATCHES_BIT = 0x00000100,
				TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT = 0x00000200,
				COMPUTE_SHADER_INVOCATIONS_BIT = 0x00000400,
				TASK_SHADER_INVOCATIONS_BIT_EXT = 0x00000800,
				MESH_SHADER_INVOCATIONS_BIT_EXT = 0x00001000
			};

			enum class ResultFlagBit
			{
				RESULT_64_BIT = 0x00000001,
				RESULT_WAIT_BIT = 0x00000002,
				RESULT_WITH_AVAILABILITY_BIT = 0x00000004,
				RESULT_PARTIAL_BIT = 0x00000008,
			};

			struct CreateInfo
			{
				std::string					debugName;
				DeviceRef					device;
				bool						reset;
				Type						type;
				uint32_t					count;
				PipelineStatisticFlagBit	pipelineStatisticFlags;
				AllocatorRef				allocatorCPU;
			};

			//Methods
		public:
			static QueryPoolRef Create(CreateInfo* pCreateInfo);
			virtual ~QueryPool() = default;
			const CreateInfo& GetCreateInfo() { return m_CI; }

			virtual void Reset(uint32_t firstQuery, uint32_t queryCount) = 0;
			virtual double ConvertTimingDataMilliseconds(uint64_t datum) = 0;

			inline const BufferRef& GetReadbackBuffer() { return m_ReadbackBuffer; }

		protected:
			void CreateReadbackBuffer();

			//Members
		protected:
			CreateInfo m_CI = {};

			BufferRef m_ReadbackBuffer;
		};
	}
}