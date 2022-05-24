#pragma once
#include "miru_core_common.h"
#include "PipelineHelper.h"

namespace miru
{
namespace base
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
	constexpr DeviceAddress DeviceAddressNull = 0;
	constexpr DeviceOrHostAddress DeviceOrHostAddressNull = { 0 };
	constexpr DeviceOrHostAddressConst DeviceOrHostAddressConstNull = { 0 };

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
			AccelerationStructureRef	srcAccelerationStructure;
			AccelerationStructureRef	dstAccelerationStructure;
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
		static AccelerationStructureBuildInfoRef Create(BuildGeometryInfo* pBuildGeometryInfo);
		virtual ~AccelerationStructureBuildInfo() = default;
		const BuildSizesInfo& GetBuildSizesInfo() { return m_BSI; }
		const BuildGeometryInfo& GetBuildGeometryInfo() { return m_BGI; }

		//Members
	protected:
		BuildSizesInfo m_BSI = {};
		BuildGeometryInfo m_BGI = {};
	};

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
			std::string		debugName;
			void*			device;
			FlagBit			flags;
			BufferRef		buffer;
			size_t			offset;
			size_t			size;
			Type			type;
			DeviceAddress	deviceAddress;
		};

		//Methods
	public:
		static AccelerationStructureRef Create(CreateInfo* pCreateInfo);
		virtual ~AccelerationStructure() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	MIRU_API DeviceAddress GetAccelerationStructureDeviceAddress(void* device, const AccelerationStructureRef& accelerationStructure);
	MIRU_API DeviceAddress GetBufferDeviceAddress(void* device, const BufferRef& buffer);
}
}