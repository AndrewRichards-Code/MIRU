#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12AccelerationStructure.h"
#include "D3D12Buffer.h"
#include "D3D12Pipeline.h"

using namespace miru;
using namespace d3d12;

AccelerationStructureBuildInfo::AccelerationStructureBuildInfo(AccelerationStructureBuildInfo::BuildGeometryInfo* pBuildGeometryInfo) 
	:m_Device(reinterpret_cast<ID3D12Device*>(pBuildGeometryInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_BGI = *pBuildGeometryInfo;

	D3D12_GPU_VIRTUAL_ADDRESS instanceDescs = 0;
	D3D12_ELEMENTS_LAYOUT instanceDescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

	m_Geometries.reserve(m_BGI.geometries.size());
	for (const auto& geometry : m_BGI.geometries)
	{
		D3D12_RAYTRACING_GEOMETRY_DESC d3d12Geometry;
		d3d12Geometry.Type = static_cast<D3D12_RAYTRACING_GEOMETRY_TYPE>(geometry.type);
		d3d12Geometry.Flags = static_cast<D3D12_RAYTRACING_GEOMETRY_FLAGS>(geometry.flags);
		switch (geometry.type)
		{
		case BuildGeometryInfo::Geometry::Type::TRIANGLES:
		{
			const BuildGeometryInfo::Geometry::TrianglesData& triangles = geometry.triangles;
			D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC& d3d12Triangles = d3d12Geometry.Triangles;

			d3d12Triangles.Transform3x4 = triangles.transformData.deviceAddress;
			d3d12Triangles.IndexFormat = triangles.indexType == crossplatform::IndexType::UINT32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			d3d12Triangles.VertexFormat = triangles.vertexFormat == crossplatform::VertexType::VEC4 ? DXGI_FORMAT_R32G32B32_FLOAT : Pipeline::ToDXGI_FORMAT(triangles.vertexFormat);
			d3d12Triangles.IndexCount = triangles.maxIndex;
			d3d12Triangles.VertexCount = triangles.maxVertex;
			d3d12Triangles.IndexBuffer = triangles.indexData.deviceAddress;
			d3d12Triangles.VertexBuffer = { triangles.vertexData.deviceAddress, triangles.vertexStride };
			break;
		}
		case BuildGeometryInfo::Geometry::Type::AABBS:
		{
			const BuildGeometryInfo::Geometry::AabbsData& aabbs = geometry.aabbs;
			D3D12_RAYTRACING_GEOMETRY_AABBS_DESC& d3d12Aabbs = d3d12Geometry.AABBs;

			d3d12Aabbs.AABBs.StartAddress = aabbs.data.deviceAddress;
			d3d12Aabbs.AABBs.StrideInBytes = aabbs.stride;
			d3d12Aabbs.AABBCount = 1;
			break;
		}
		case BuildGeometryInfo::Geometry::Type::INSTANCES:
		{
			const BuildGeometryInfo::Geometry::InstancesData& instances = geometry.instances;
			instanceDescs = instances.data.deviceAddress;
			instanceDescsLayout = instances.arrayOfPointers ? D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS : D3D12_ELEMENTS_LAYOUT_ARRAY;
			break;
		}
		default:
		{
			MIRU_ASSERT(true, "ERROR: D3D12: Unknown BuildGeometryInfo::Geometry::Type.");
			break;
		}
		}
		m_Geometries.push_back(d3d12Geometry);
		
		//We only deal with one InstanceDesc
		if (geometry.type == BuildGeometryInfo::Geometry::Type::INSTANCES)
			break;
	}

	m_BRASI.Type = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE>(m_BGI.type);
	m_BRASI.Flags = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(m_BGI.flags);
	m_BRASI.Flags |= (m_BGI.mode == BuildGeometryInfo::Mode::UPDATE ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE);
	if (!m_Geometries.empty())
	{
		m_BRASI.NumDescs = static_cast<uint32_t>(m_Geometries.size());
		m_BRASI.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		m_BRASI.pGeometryDescs = m_Geometries.data();
	}
	else
	{
		m_BRASI.NumDescs = 1;
		m_BRASI.DescsLayout = instanceDescsLayout;
		m_BRASI.InstanceDescs = instanceDescs;
	}

	m_RASPBI.ResultDataMaxSizeInBytes = 0;
	m_RASPBI.ScratchDataSizeInBytes = 0;
	m_RASPBI.UpdateScratchDataSizeInBytes = 0;
	reinterpret_cast<ID3D12Device5*>(m_Device)->GetRaytracingAccelerationStructurePrebuildInfo(&m_BRASI, &m_RASPBI);

	//Crossplatform assignment
	m_BSI.accelerationStructureSize = m_RASPBI.ResultDataMaxSizeInBytes;
	m_BSI.updateScratchSize			= m_RASPBI.UpdateScratchDataSizeInBytes;
	m_BSI.buildScratchSize			= m_RASPBI.ScratchDataSizeInBytes;
}
AccelerationStructureBuildInfo::~AccelerationStructureBuildInfo()
{
	MIRU_CPU_PROFILE_FUNCTION();
}

AccelerationStructure::AccelerationStructure(AccelerationStructure::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	m_SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	m_SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_SRVDesc.RaytracingAccelerationStructure.Location = GetBufferDeviceAddress();
}

AccelerationStructure::~AccelerationStructure() 
{
	MIRU_CPU_PROFILE_FUNCTION();
}

#endif