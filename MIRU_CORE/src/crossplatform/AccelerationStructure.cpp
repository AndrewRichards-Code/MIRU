#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "directx12/D3D12AccelerationStructure.h"
#include "directx12/D3D12Buffer.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKAccelerationStructure.h"
#include "vulkan/VKBuffer.h"
#endif

using namespace miru;
using namespace crossplatform;

Ref<AccelerationStructureBuildInfo> AccelerationStructureBuildInfo::Create(AccelerationStructureBuildInfo::BuildGeometryInfo* pBuildGeometryInfo)
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
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

Ref<AccelerationStructure> AccelerationStructure::Create(AccelerationStructure::CreateInfo* pCreateInfo)
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
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

DeviceAddress AccelerationStructure::GetBufferDeviceAddress()
{
	return crossplatform::GetBufferDeviceAddress(m_CI.device, m_CI.buffer);
}

DeviceAddress miru::crossplatform::GetBufferDeviceAddress(void* device, const Ref<Buffer>& buffer)
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
		return vkGetBufferDeviceAddress(*reinterpret_cast<VkDevice*>(device), &info);
		#else
		return 0;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return 0;
	}
}