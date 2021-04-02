#pragma once
#if defined(MIRU_D3D12)
#include "crossplatform/AccelerationStructure.h"

namespace miru
{
namespace d3d12
{
	class AccelerationStructureBuildInfo final : public crossplatform::AccelerationStructureBuildInfo
	{
		//Methods
	public:
		AccelerationStructureBuildInfo(BuildGeometryInfo* pBuildGeometryInfo);
		~AccelerationStructureBuildInfo();

		//Members
	public:
		ID3D12Device* m_Device;

		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_Geometries;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS m_BRASI;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO m_RASPBI;
	};

	class AccelerationStructure final : public crossplatform::AccelerationStructure
	{
		//Methods
	public:
		AccelerationStructure(AccelerationStructure::CreateInfo* pCreateInfo);
		~AccelerationStructure();

		//Members
	public:
		ID3D12Device* m_Device;
	};
}
}
#endif