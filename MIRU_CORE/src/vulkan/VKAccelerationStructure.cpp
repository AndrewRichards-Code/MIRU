#include "VKAccelerationStructure.h"
#include "VKBuffer.h"
#include "VKPipeline.h"

using namespace miru;
using namespace vulkan;

AccelerationStructureBuildInfo::AccelerationStructureBuildInfo(AccelerationStructureBuildInfo::BuildGeometryInfo* pBuildGeometryInfo) 
	:m_Device(*reinterpret_cast<VkDevice*>(pBuildGeometryInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_BGI = *pBuildGeometryInfo;

	m_Geometries.reserve(m_BGI.geometries.size());
	for (const auto& geometry : m_BGI.geometries)
	{
		VkAccelerationStructureGeometryKHR vkGeometry;
		vkGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		vkGeometry.pNext = nullptr;
		vkGeometry.geometryType = static_cast<VkGeometryTypeKHR>(geometry.type);
		switch (geometry.type)
		{
		case BuildGeometryInfo::Geometry::Type::TRIANGLES:
		{
			const BuildGeometryInfo::Geometry::TrianglesData& triangles = geometry.triangles;
			VkAccelerationStructureGeometryTrianglesDataKHR& vkTriangles = vkGeometry.geometry.triangles;

			vkTriangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			vkTriangles.pNext = nullptr;
			vkTriangles.vertexFormat = Pipeline::ToVkFormat(triangles.vertexFormat);
			vkTriangles.vertexData = *(reinterpret_cast<const VkDeviceOrHostAddressConstKHR*>(&triangles.vertexData));
			vkTriangles.vertexStride = triangles.vertexStride;
			vkTriangles.maxVertex = triangles.maxVertex;
			vkTriangles.indexType = static_cast<VkIndexType>(triangles.indexType);
			vkTriangles.indexData = *(reinterpret_cast<const VkDeviceOrHostAddressConstKHR*>(&triangles.indexData));
			vkTriangles.transformData = *(reinterpret_cast<const VkDeviceOrHostAddressConstKHR*>(&triangles.transformData));
			break;
		}
		case BuildGeometryInfo::Geometry::Type::AABBS:
		{
			const BuildGeometryInfo::Geometry::AabbsData& aabbs = geometry.aabbs;
			VkAccelerationStructureGeometryAabbsDataKHR& vkAabbs = vkGeometry.geometry.aabbs;

			vkAabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
			vkAabbs.pNext = nullptr;
			vkAabbs.data = *(reinterpret_cast<const VkDeviceOrHostAddressConstKHR*>(&aabbs.data));
			vkAabbs.stride = aabbs.stride;
			break;
		}
		case BuildGeometryInfo::Geometry::Type::INSTANCES:
		{
			const BuildGeometryInfo::Geometry::InstancesData& instances = geometry.instances;
			VkAccelerationStructureGeometryInstancesDataKHR& vkInstances = vkGeometry.geometry.instances;

			vkInstances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
			vkInstances.pNext = nullptr;
			vkInstances.arrayOfPointers = instances.arrayOfPointers;
			vkInstances.data = *(reinterpret_cast<const VkDeviceOrHostAddressConstKHR*>(&instances.data));
			break;
		}
		default:
		{
			MIRU_FATAL(true, "ERROR: VULKAN: Unknown BuildGeometryInfo::Geometry::Type.");
			break;
		}
		}
		vkGeometry.flags = static_cast<VkGeometryFlagsKHR>(geometry.flags);
		m_Geometries.push_back(vkGeometry);
	}

	m_ASBGI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	m_ASBGI.pNext = nullptr;
	m_ASBGI.type = static_cast<VkAccelerationStructureTypeKHR>(m_BGI.type);
	m_ASBGI.flags = static_cast<VkBuildAccelerationStructureFlagsKHR>(m_BGI.flags);
	m_ASBGI.mode = static_cast<VkBuildAccelerationStructureModeKHR>(m_BGI.mode);
	m_ASBGI.srcAccelerationStructure = m_BGI.srcAccelerationStructure ? ref_cast<AccelerationStructure>(m_BGI.srcAccelerationStructure)->m_AS : VK_NULL_HANDLE;
	m_ASBGI.dstAccelerationStructure = m_BGI.dstAccelerationStructure ? ref_cast<AccelerationStructure>(m_BGI.dstAccelerationStructure)->m_AS : VK_NULL_HANDLE;
	m_ASBGI.geometryCount = static_cast<uint32_t>(m_Geometries.size());
	m_ASBGI.pGeometries = m_Geometries.data();
	m_ASBGI.ppGeometries = nullptr;
	m_ASBGI.scratchData = *(reinterpret_cast<VkDeviceOrHostAddressKHR*>(&m_BGI.scratchData));

	m_ASBSI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	m_ASBSI.pNext = nullptr;
	m_ASBSI.accelerationStructureSize = 0;
	m_ASBSI.updateScratchSize = 0;
	m_ASBSI.buildScratchSize = 0;
	vkGetAccelerationStructureBuildSizesKHR(m_Device, static_cast<VkAccelerationStructureBuildTypeKHR>(m_BGI.buildType), &m_ASBGI, &m_BGI.maxPrimitiveCounts, &m_ASBSI);

	//Crossplatform assignment
	m_BSI.accelerationStructureSize = m_ASBSI.accelerationStructureSize;
	m_BSI.updateScratchSize			= m_ASBSI.updateScratchSize;
	m_BSI.buildScratchSize			= m_ASBSI.buildScratchSize;
}
AccelerationStructureBuildInfo::~AccelerationStructureBuildInfo()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

AccelerationStructure::AccelerationStructure(AccelerationStructure::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;
	
	m_ASCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	m_ASCI.pNext = nullptr;
	m_ASCI.createFlags = static_cast<VkAccelerationStructureCreateFlagsKHR>(m_CI.flags);
	m_ASCI.buffer = ref_cast<Buffer>(m_CI.buffer)->m_Buffer;
	m_ASCI.offset = m_CI.offset;
	m_ASCI.size = m_CI.size;
	m_ASCI.type = static_cast<VkAccelerationStructureTypeKHR>(m_CI.type);
	m_ASCI.deviceAddress = m_CI.deviceAddress;

	MIRU_FATAL(vkCreateAccelerationStructureKHR(m_Device, &m_ASCI, nullptr, &m_AS), "ERROR: VULKAN: Failed to create AccelerationStructure.");
	VKSetName<VkAccelerationStructureKHR>(m_Device, m_AS, m_CI.debugName);
}

AccelerationStructure::~AccelerationStructure() 
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyAccelerationStructureKHR(m_Device, m_AS, nullptr);
}