#pragma once
#if defined(MIRU_VULKAN)
#include "base/AccelerationStructure.h"

namespace miru
{
namespace vulkan
{
	class AccelerationStructureBuildInfo final : public base::AccelerationStructureBuildInfo
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

	class AccelerationStructure final : public base::AccelerationStructure
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