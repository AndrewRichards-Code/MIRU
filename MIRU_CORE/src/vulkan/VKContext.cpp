#include "miru_core_common.h"
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

Context::Context(Context::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//Instance
	uint32_t apiVersion = VK_MAKE_VERSION(m_CI.api_version_major, m_CI.api_version_minor, 0);

	m_AI.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	m_AI.pNext = nullptr;
	m_AI.pApplicationName = m_CI.applicationName;
	m_AI.applicationVersion = apiVersion;
	m_AI.pEngineName = "MIRU - x64";
	m_AI.engineVersion = apiVersion;
	m_AI.apiVersion = apiVersion;

	//Add additional instance/device layers/extensions
	auto push_back_exclusive = [](std::vector<std::string>& list, const std::string& string) -> void
	{
		bool found = false;
		for (auto& item : list)
		{
			if (strcmp(string.c_str(), item.c_str()) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			list.push_back(string);
	};

#if defined(_DEBUG)
	push_back_exclusive(m_CI.instanceLayers, "VK_LAYER_LUNARG_standard_validation");
	push_back_exclusive(m_CI.deviceLayers, "VK_LAYER_LUNARG_standard_validation");
#endif

	push_back_exclusive(m_CI.instanceExtensions, "VK_KHR_surface");
	push_back_exclusive(m_CI.deviceExtensions, "VK_KHR_swapchain");

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	push_back_exclusive(m_CI.instanceExtensions, "VK_KHR_win32_surface");
#elif defined(VK_USE_PLATFORM_ANDROIOD_KHR)
	push_back_exclusive(m_CI.instanceExtensions, "VK_KHR_android_surface");
#endif

	uint32_t layerCount = 0;
	MIRU_ASSERT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	std::vector<VkLayerProperties> instanceLayerProperties;
	instanceLayerProperties.resize(layerCount);
	MIRU_ASSERT(vkEnumerateInstanceLayerProperties(&layerCount, instanceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	for (auto& requestLayer : m_CI.instanceLayers)
	{
		for (auto& layerProperty : instanceLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveInstanceLayers.push_back(requestLayer.c_str()); break;
		}
	}

	if (GraphicsAPI::IsSetNameAllowed())
		push_back_exclusive(m_CI.instanceExtensions, "VK_EXT_debug_utils");

	uint32_t extensionCount = 0;
	MIRU_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	std::vector<VkExtensionProperties> instanceExtensionProperties;
	instanceExtensionProperties.resize(extensionCount);
	MIRU_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	for (auto& requestExtension : m_CI.instanceExtensions)
	{
		for (auto& extensionProperty : instanceExtensionProperties)
		{
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				m_ActiveInstanceExtensions.push_back(requestExtension.c_str()); break;
		}
	}

	m_InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	m_InstanceCI.pNext = nullptr;
	m_InstanceCI.flags = 0;
	m_InstanceCI.pApplicationInfo = &m_AI;
	m_InstanceCI.enabledLayerCount = static_cast<uint32_t>(m_ActiveInstanceLayers.size());
	m_InstanceCI.ppEnabledLayerNames = m_ActiveInstanceLayers.data();
	m_InstanceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveInstanceExtensions.size());
	m_InstanceCI.ppEnabledExtensionNames = m_ActiveInstanceExtensions.data();

	MIRU_ASSERT(vkCreateInstance(&m_InstanceCI, nullptr, &m_Instance), "ERROR: VULKAN: Failed to create Instance.");

	//PhysicalDevice
	m_PhysicalDevices = PhysicalDevices(m_Instance);

	//Devices
	VkPhysicalDevice physicalDevice = m_PhysicalDevices.m_PhysicalDevices[0]; //We only use the first PhysicalDevice
	VkPhysicalDeviceFeatures physicalDeviceFeatures = m_PhysicalDevices.m_PhysicalDeviceFeatures[0];

	uint32_t queueFamilyPropertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, nullptr);
	m_QueueFamilyProperties.resize(queueFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, m_QueueFamilyProperties.data());

	m_QueuePriorities.resize(m_QueueFamilyProperties.size());
	m_DeviceQueueCIs.resize(m_QueueFamilyProperties.size());
	for (size_t i = 0; i < m_DeviceQueueCIs.size(); i++)
	{
		for (size_t j = 0; j < m_QueueFamilyProperties[i].queueCount; j++)
			m_QueuePriorities[i].push_back(1.0f);

		m_DeviceQueueCIs[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		m_DeviceQueueCIs[i].pNext = nullptr;
		m_DeviceQueueCIs[i].flags = 0;
		m_DeviceQueueCIs[i].queueFamilyIndex = static_cast<uint32_t>(i);
		m_DeviceQueueCIs[i].queueCount = m_QueueFamilyProperties[i].queueCount;
		m_DeviceQueueCIs[i].pQueuePriorities = m_QueuePriorities[i].data();
	}

	uint32_t layerPropertiesCount = 0;
	MIRU_ASSERT(vkEnumerateDeviceLayerProperties(physicalDevice, &layerPropertiesCount, nullptr), "ERROR: VULKAN: Failed to enumerate DeviceLayerProperties.");
	std::vector<VkLayerProperties> deviceLayerProperties;
	deviceLayerProperties.resize(layerPropertiesCount);
	MIRU_ASSERT(vkEnumerateDeviceLayerProperties(physicalDevice, &layerPropertiesCount, deviceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceLayerProperties.");
	for (auto& requestLayer : m_CI.deviceLayers)
	{
		for (auto& layerProperty : deviceLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveDeviceLayers.push_back(requestLayer.c_str()); break;
		}
	}

	uint32_t extensionPropertiesCount = 0;
	MIRU_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionPropertiesCount, 0), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	std::vector<VkExtensionProperties> deviceExtensionProperties;
	deviceExtensionProperties.resize(extensionPropertiesCount);
	MIRU_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionPropertiesCount, deviceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	for (auto& requestExtension : m_CI.deviceExtensions)
	{
		for (auto& extensionProperty : deviceExtensionProperties)
		{
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				m_ActiveDeviceExtensions.push_back(requestExtension.c_str()); break;
		}
	}

	m_DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	m_DeviceCI.pNext = nullptr;
	m_DeviceCI.flags = 0;
	m_DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(m_DeviceQueueCIs.size());
	m_DeviceCI.pQueueCreateInfos = m_DeviceQueueCIs.data();
	m_DeviceCI.enabledLayerCount = static_cast<uint32_t>(m_ActiveDeviceLayers.size());
	m_DeviceCI.ppEnabledLayerNames = m_ActiveDeviceLayers.data();
	m_DeviceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveDeviceExtensions.size());
	m_DeviceCI.ppEnabledExtensionNames = m_ActiveDeviceExtensions.data();
	m_DeviceCI.pEnabledFeatures = &physicalDeviceFeatures;

	MIRU_ASSERT(vkCreateDevice(physicalDevice, &m_DeviceCI, nullptr, &m_Device), "ERROR: VULKAN: Failed to create Device");

	vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(m_Device, "vkSetDebugUtilsObjectNameEXT");
	VKSetName<VkInstance>(m_Device, (uint64_t)m_Instance, (std::string(m_AI.pEngineName) + " - VkInstance").c_str());
	VKSetName<VkPhysicalDevice>(m_Device, (uint64_t)m_PhysicalDevices.m_PhysicalDevices[0], m_PhysicalDevices.m_PhysicalDeviceProperties[0].deviceName);
	VKSetName<VkDevice>(m_Device, (uint64_t)m_Device, m_CI.deviceDebugName);
	
	for (size_t i = 0; i < m_DeviceQueueCIs.size(); i++)
	{
		std::vector<VkQueue>localQueues;
		for (size_t j = 0; j < m_DeviceQueueCIs[i].queueCount; j++)
		{
			VkQueue queue;
			vkGetDeviceQueue(m_Device, static_cast<uint32_t>(i), static_cast<uint32_t>(j), &queue);
			localQueues.push_back(queue);
			
			VkQueueFlags flags = m_QueueFamilyProperties[i].queueFlags;
			std::string typeStr("");
			flags & VK_QUEUE_GRAPHICS_BIT ? typeStr + "Graphics/" : + "";
			flags & VK_QUEUE_COMPUTE_BIT ? typeStr + "Compute/" : + "";
			flags & VK_QUEUE_TRANSFER_BIT ? typeStr + "Transfer " : + "";
			VKSetName<VkQueue>(m_Device, (uint64_t)queue, (std::string(m_CI.deviceDebugName) + ": Queue - " + typeStr).c_str());
		}
		m_Queues.push_back(localQueues);
		localQueues.clear();
	}
}

Context::~Context()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

Context::PhysicalDevices::PhysicalDevices(const VkInstance& instance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	uint32_t physicalDeviceCount = 0;
	MIRU_ASSERT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
	m_PhysicalDevices.resize(physicalDeviceCount);
	MIRU_ASSERT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, m_PhysicalDevices.data()), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");

	for (size_t i = 0; i < m_PhysicalDevices.size(); i++)
	{
		if (m_PhysicalDevices[i] == VK_NULL_HANDLE)
		{
			std::vector<VkPhysicalDevice>::const_iterator it = m_PhysicalDevices.cbegin() + i;
			m_PhysicalDevices.erase(it);
		}
	}
	if (m_PhysicalDevices.empty())
	{
		MIRU_ASSERT(true, "ERROR: VULKAN: No valid Vulkan devices are available.");
	}

	m_PhysicalDeviceFeatures.resize(m_PhysicalDevices.size());
	m_PhysicalDeviceProperties.resize(m_PhysicalDevices.size());
	m_PhysicalDeviceMemoryProperties.resize(m_PhysicalDevices.size());
	for (size_t i = 0; i < m_PhysicalDevices.size(); i++)
	{
		vkGetPhysicalDeviceFeatures(m_PhysicalDevices[i], &m_PhysicalDeviceFeatures[i]);
		vkGetPhysicalDeviceProperties(m_PhysicalDevices[i], &m_PhysicalDeviceProperties[i]);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevices[i], &m_PhysicalDeviceMemoryProperties[i]);
	}
}
