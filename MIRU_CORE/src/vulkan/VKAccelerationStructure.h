#pragma once
#if defined(MIRU_VULKAN)
#include "crossplatform/AccelerationStructure.h"

namespace miru
{
namespace vulkan
{
	class AccelerationStructureBuildInfo final : public crossplatform::AccelerationStructureBuildInfo
	{
		//Methods
	public:
		AccelerationStructureBuildInfo(BuildGeometryInfo* pBuildGeometryInfo);
		~AccelerationStructureBuildInfo();

		//Members
	public:
		VkDevice& m_Device;

		std::vector<VkAccelerationStructureGeometryKHR> m_Geometries;
		VkAccelerationStructureBuildGeometryInfoKHR m_ASBGI;
		VkAccelerationStructureBuildSizesInfoKHR m_ASBSI;
	};

	class AccelerationStructure final : public crossplatform::AccelerationStructure
	{
		//Methods
	public:
		AccelerationStructure(AccelerationStructure::CreateInfo* pCreateInfo);
		~AccelerationStructure();

		//Members
	public:
		VkDevice& m_Device;

		VkAccelerationStructureKHR m_AS;
		VkAccelerationStructureCreateInfoKHR m_ASCI;
	};
}
}
#endif