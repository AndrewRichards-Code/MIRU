#pragma once
#include "base/AccelerationStructure.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class AccelerationStructureBuildInfo final : public base::AccelerationStructureBuildInfo
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

	class AccelerationStructure final : public base::AccelerationStructure
	{
		//Methods
	public:
		AccelerationStructure(AccelerationStructure::CreateInfo* pCreateInfo);
		~AccelerationStructure();

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc;

		D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescHandle = {};
	};
}
}