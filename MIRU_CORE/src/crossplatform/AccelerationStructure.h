#pragma once
#include "miru_core_common.h"
#include "PipelineHelper.h"

namespace miru
{
namespace crossplatform
{
	typedef uint64_t DeviceAddress;
	union DeviceOrHostAddress
	{
		DeviceAddress	deviceAddress;
		void*			hostAddress;
	};
	union DeviceOrHostAddressConst
	{
		DeviceAddress	deviceAddress;
		const void*		hostAddress;
	};
	#define MIRU_NULL_DEVICE_ADDRESS  0
	#define MIRU_NULL_DEVICE_OR_HOST_ADDRESS { 0 }
	#define MIRU_NULL_DEVICE_OR_HOST_ADDRESS_CONST MIRU_NULL_DEVICE_OR_HOST_ADDRESS

	struct StridedDeviceAddressRegion
	{
		DeviceAddress	deviceAddress;
		uint64_t		stride;
		uint64_t		size;
	};

	struct TransformMatrix
	{
		float matrix[3][4];
	};
	struct AabbData
	{
		float minX;
		float minY;
		float minZ;
		float maxX;
		float maxY;
		float maxZ;
	};
	enum class InstanceDataFlagBit : uint32_t
	{
		NONE_BIT = 0x00000000,
		TRIANGLE_FACING_CULL_DISABLE_BIT = 0x00000001,
		TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT = 0x00000002,
		FORCE_OPAQUE_BIT = 0x00000004,
		FORCE_NO_OPAQUE_BIT = 0x00000008,
	};
	struct InstanceData
	{
		TransformMatrix		transform;
		uint32_t			instanceCustomIndex : 24;
		uint32_t			mask : 8;
		uint32_t			instanceShaderBindingTableRecordOffset : 24;
		uint32_t			flags : 8;
		DeviceAddress		accelerationStructureReference;
	};

	class AccelerationStructure;
	class MIRU_API AccelerationStructureBuildInfo
	{
		//enums/structs
	public:
		struct BuildGeometryInfo
		{
			enum class Type : uint32_t
			{
				TOP_LEVEL,
				BOTTOM_LEVEL,
				GENERIC,
			};
			enum class FlagBit : uint32_t
			{
				NONE_BIT = 0x00000000,
				ALLOW_UPDATE_BIT = 0x00000001,
				ALLOW_COMPACTION_BIT = 0x00000002,
				PREFER_FAST_TRACE_BIT = 0x00000004,
				PREFER_FAST_BUILD_BIT = 0x00000008,
				LOW_MEMORY_BIT = 0x00000010,
			};
			enum class Mode : uint32_t
			{
				BUILD,
				UPDATE
			};
			struct Geometry
			{
				struct TrianglesData
				{
					VertexType					vertexFormat;
					DeviceOrHostAddressConst	vertexData;
					uint64_t					vertexStride;
					uint32_t					maxVertex;
					IndexType					indexType;
					DeviceOrHostAddressConst	indexData;
					uint32_t					maxIndex;
					DeviceOrHostAddressConst	transformData;
				};
				struct AabbsData
				{
					DeviceOrHostAddressConst	data;
					uint64_t					stride;
				};
				struct InstancesData
				{
					bool						arrayOfPointers;
					DeviceOrHostAddressConst	data;
				};
				enum class Type : uint32_t
				{
					TRIANGLES,
					AABBS,
					INSTANCES
				};
				enum class FlagBit : uint32_t
				{
					NONE_BIT = 0x00000000,
					OPAQUE_BIT = 0x00000001,
					NO_DUPLICATE_ANY_HIT_INVOCATION_BIT = 0x00000002,
				};

				Type				type;
				union
				{
					TrianglesData	triangles;
					AabbsData		aabbs;
					InstancesData	instances;
				};
				FlagBit				flags;
			};
			enum class BuildType : uint32_t
			{
				HOST,
				DEVICE,
				HOST_DEVICE
			};

			void*						device;
			Type						type;
			FlagBit						flags;
			Mode						mode;
			Ref<AccelerationStructure>	srcAccelerationStructure;
			Ref<AccelerationStructure>	dstAccelerationStructure;
			std::vector<Geometry>		geometries;
			DeviceOrHostAddress			scratchData;
			BuildType					buildType;
			uint32_t					maxPrimitiveCounts;
		};
		struct BuildSizesInfo
		{
			uint64_t accelerationStructureSize;
			uint64_t updateScratchSize;
			uint64_t buildScratchSize;
		};
		struct BuildRangeInfo
		{
			uint32_t primitiveCount;
			uint32_t primitiveOffset;
			uint32_t firstVertex;
			uint32_t transformOffset;
		};

		//Methods
	public:
		static Ref<AccelerationStructureBuildInfo> Create(BuildGeometryInfo* pBuildGeometryInfo);
		virtual ~AccelerationStructureBuildInfo() = default;
		const BuildSizesInfo& GetBuildSizesInfo() { return m_BSI; }
		const BuildGeometryInfo& GetBuildGeometryInfo() { return m_BGI; }

		//Members
	protected:
		BuildSizesInfo m_BSI= {};
		BuildGeometryInfo m_BGI = {};
	};
	MIRU_CLASS_REF_TYPEDEF(AccelerationStructureBuildInfo);

	class Buffer;
	class MIRU_API AccelerationStructure
	{
		//enums/structs
	public:
		enum class Type : uint32_t
		{
			TOP_LEVEL,
			BOTTOM_LEVEL,
			GENERIC,
		};
		enum class FlagBit : uint32_t
		{
			NONE_BIT = 0x00000000,
			DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = 0x00000001
		};
		
		struct CreateInfo
		{
			std::string			debugName;
			void*				device;
			FlagBit				flags;
			Ref<Buffer>			buffer;
			size_t				offset;
			size_t				size;
			Type				type;
			DeviceAddress		deviceAddress;
		};

		//Methods
	public:
		static Ref<AccelerationStructure> Create(CreateInfo* pCreateInfo);
		virtual ~AccelerationStructure() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
	MIRU_CLASS_REF_TYPEDEF(AccelerationStructure);

	MIRU_API DeviceAddress GetAccelerationStructureDeviceAddress(void* device, const Ref<AccelerationStructure>& accelerationStructure);
	MIRU_API DeviceAddress GetBufferDeviceAddress(void* device, const Ref<Buffer>& buffer);
}
}