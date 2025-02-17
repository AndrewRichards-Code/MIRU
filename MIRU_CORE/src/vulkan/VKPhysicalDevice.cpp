#include "vulkan/VKPhysicalDevice.h"
#include "vulkan/VKInstance.h"

using namespace miru;
using namespace vulkan;

PhysicalDevice::PhysicalDevice(CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	InstanceRef instance = ref_cast<Instance>(GetInstance());
	m_PhysicalDevice = reinterpret_cast<VkPhysicalDevice>(m_CI.nativeHandle);

	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

	//OpenXR Data
	Instance::OpenXRVulkanData* openXRVulkanData = reinterpret_cast<Instance::OpenXRVulkanData*>(instance->GetCreateInfo().pNext);
	if (!(openXRVulkanData && openXRVulkanData->type == Instance::CreateInfoExtensionStructureTypes::OPENXR_VULKAN_DATA))
		openXRVulkanData = nullptr;

	if (openXRVulkanData)
	{
		VkPhysicalDevice openxrPhysicalDevice = openXRVulkanData->getPhysicalDeviceVulkan(instance->m_Instance);
		m_OpenXRValid = m_PhysicalDevice == openxrPhysicalDevice;
	}
}

PhysicalDevice::~PhysicalDevice()
{
	MIRU_CPU_PROFILE_FUNCTION();
}