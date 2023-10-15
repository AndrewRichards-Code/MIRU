#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12AccelerationStructure.h"
#include "d3d12/D3D12Buffer.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKAccelerationStructure.h"
#include "vulkan/VKBuffer.h"
#endif

using namespace miru;
using namespace base;

AccelerationStructureBuildInfoRef AccelerationStructureBuildInfo::Create(AccelerationStructureBuildInfo::BuildGeometryInfo* pBuildGeometryInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::AccelerationStructureBuildInfo>(pBuildGeometryInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::AccelerationStructureBuildInfo>(pBuildGeometryInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

AccelerationStructureRef AccelerationStructure::Create(AccelerationStructure::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::AccelerationStructure>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::AccelerationStructure>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

DeviceAddress miru::base::GetAccelerationStructureDeviceAddress(void* device, const AccelerationStructureRef& accelerationStructure)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return ref_cast<d3d12::Buffer>(accelerationStructure->GetCreateInfo().buffer)->m_Buffer->GetGPUVirtualAddress();
		#else	
		return 0;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		VkAccelerationStructureDeviceAddressInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		info.pNext = nullptr;
		info.accelerationStructure = ref_cast<vulkan::AccelerationStructure>(accelerationStructure)->m_AS;
		return vulkan::vkGetAccelerationStructureDeviceAddressKHR(*reinterpret_cast<VkDevice*>(device), &info);
		#else
		return 0;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return 0;
	}
}

DeviceAddress miru::base::GetBufferDeviceAddress(void* device, const BufferRef& buffer)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return ref_cast<d3d12::Buffer>(buffer)->m_Buffer->GetGPUVirtualAddress();
		#else
		return 0;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		VkBufferDeviceAddressInfo info;
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		info.pNext = nullptr;
		info.buffer = ref_cast<vulkan::Buffer>(buffer)->m_Buffer;
		return vulkan::vkGetBufferDeviceAddress(*reinterpret_cast<VkDevice*>(device), &info);
		#else
		return 0;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return 0;
	}
}