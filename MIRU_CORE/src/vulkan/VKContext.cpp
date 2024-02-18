#include "VKContext.h"
#include <sstream>

using namespace miru;
using namespace vulkan;

Context::Context(Context::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//OpenXR Data
	OpenXRVulkanData* openXRVulkanData = reinterpret_cast<OpenXRVulkanData*>(m_CI.pNext);
	if (!(openXRVulkanData && openXRVulkanData->type == CreateInfoExtensionStructureTypes::OPENXR_VULKAN_DATA))
		openXRVulkanData = nullptr;

	//Instance
	uint32_t apiVersion = VK_API_VERSION_1_0;
	PFN_vkEnumerateInstanceVersion _vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
	if (_vkEnumerateInstanceVersion != nullptr)
		_vkEnumerateInstanceVersion(&apiVersion);

	if (openXRVulkanData)
	{
		uint16_t apiVersionMajor = VK_API_VERSION_MAJOR(apiVersion);
		uint16_t apiVersionMinor = VK_API_VERSION_MINOR(apiVersion);

		bool underMin = apiVersionMajor < openXRVulkanData->minApiVersionMajorSupported
			|| apiVersionMinor < openXRVulkanData->minApiVersionMinorSupported;
		bool overMax = apiVersionMajor > openXRVulkanData->maxApiVersionMajorSupported
			|| apiVersionMinor > openXRVulkanData->maxApiVersionMinorSupported;

		if (underMin)
		{
			MIRU_FATAL(true, "ERROR: VULKAN: Selected API Version is less than the minimum for OpenXR.");
		}
		if (overMax)
		{
			MIRU_WARN(true, "WARN: VULKAN: Selected API Version is greater than the maximum for OpenXR.");
		}
	}
	
	#if defined(VK_USE_PLATFORM_WIN32_KHR)
	const char* engineName = "MIRU - x64";
	#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	const char* engineName = "MIRU - ARM64";
	#endif

	m_AI.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	m_AI.pNext = nullptr;
	m_AI.pApplicationName = m_CI.applicationName.c_str();
	m_AI.applicationVersion = 1;
	m_AI.pEngineName = engineName;
	m_AI.engineVersion = 1;
	m_AI.apiVersion = apiVersion;

	//Add additional instance/device layers/extensions
	{
		//Debug
		if (m_CI.debugValidationLayers)
		{
			m_InstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
			m_InstanceLayers.push_back("VK_LAYER_KHRONOS_synchronization2");
			m_DeviceLayers.push_back("VK_LAYER_KHRONOS_validation");
			m_DeviceLayers.push_back("VK_LAYER_KHRONOS_synchronization2");
		}
		if (base::GraphicsAPI::IsSetNameAllowed())
			m_InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		//Surface and Swapchain
		m_InstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		m_InstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		m_InstanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
		#endif

		//Extensions
		AddExtensions();

		//OpenXR Extension
		if (openXRVulkanData)
		{
			for (const auto& instanceExtension : openXRVulkanData->instanceExtensions)
				m_InstanceExtensions.push_back(instanceExtension);

			for (const auto& deviceExtension : openXRVulkanData->deviceExtensions)
				m_DeviceExtensions.push_back(deviceExtension);
		}
	}

	uint32_t instanceLayerCount = 0;
	MIRU_FATAL(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	std::vector<VkLayerProperties> instanceLayerProperties;
	instanceLayerProperties.resize(instanceLayerCount);
	MIRU_FATAL(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	for (auto& requestLayer : m_InstanceLayers)
	{
		for (auto& layerProperty : instanceLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveInstanceLayers.push_back(requestLayer.c_str()); break;
		}
	}
	uint32_t instanceExtensionCount = 0;
	MIRU_FATAL(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	std::vector<VkExtensionProperties> instanceExtensionProperties;
	instanceExtensionProperties.resize(instanceExtensionCount);
	MIRU_FATAL(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	for (auto& requestExtension : m_InstanceExtensions)
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

	MIRU_FATAL(vkCreateInstance(&m_InstanceCI, nullptr, &m_Instance), "ERROR: VULKAN: Failed to create Instance.");

	//Load Instance Extension PFN
	LoadInstanceExtensionPFNs();

	//Debug Messenger Callback
	if (IsActive(m_ActiveInstanceExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		m_DebugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		m_DebugUtilsMessengerCI.pNext = nullptr;
		m_DebugUtilsMessengerCI.flags = 0;
		m_DebugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		m_DebugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		m_DebugUtilsMessengerCI.pfnUserCallback = MessageCallbackFunction;
		m_DebugUtilsMessengerCI.pUserData = this;

		vkCreateDebugUtilsMessengerEXT(m_Instance, &m_DebugUtilsMessengerCI, nullptr, &m_DebugUtilsMessenger);
	}

	//PhysicalDevice
	m_PhysicalDevices = PhysicalDevices(m_Instance);
	m_PhysicalDeviceIndex = 0;

	VkPhysicalDevice physicalDevice = m_PhysicalDevices.m_PDIs[m_PhysicalDeviceIndex].m_PhysicalDevice; //We only use the first PhysicalDevice
	if (openXRVulkanData)
	{
		physicalDevice = openXRVulkanData->getPhysicalDeviceVulkan(m_Instance);

		for (size_t i = 0; i < m_PhysicalDevices.m_PDIs.size(); i++)
		{
			if (m_PhysicalDevices.m_PDIs[m_PhysicalDeviceIndex].m_PhysicalDevice == physicalDevice)
			{
				m_PhysicalDeviceIndex = i;
				break;
			}
		}
	}

	//Device
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
	uint32_t deviceLayerCount = 0;
	MIRU_FATAL(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, nullptr), "ERROR: VULKAN: Failed to enumerate DeviceLayerProperties.");
	std::vector<VkLayerProperties> deviceLayerProperties;
	deviceLayerProperties.resize(deviceLayerCount);
	MIRU_FATAL(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, deviceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceLayerProperties.");
	for (auto& requestLayer : m_DeviceLayers)
	{
		for (auto& layerProperty : deviceLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveDeviceLayers.push_back(requestLayer.c_str()); break;
		}
	}
	uint32_t deviceExtensionCount = 0;
	MIRU_FATAL(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, 0), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	std::vector<VkExtensionProperties> deviceExtensionProperties;
	deviceExtensionProperties.resize(deviceExtensionCount);
	MIRU_FATAL(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, deviceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	for (auto& requestExtension : m_DeviceExtensions)
	{
		for (auto& extensionProperty : deviceExtensionProperties)
		{
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				m_ActiveDeviceExtensions.push_back(requestExtension.c_str()); break;
		}
	}

	//PhysicalDevice Features
	m_PhysicalDevices.FillOutFeaturesAndProperties(this);
	VkPhysicalDeviceFeatures* physicalDeviceFeatures = &m_PhysicalDevices.m_PDIs[m_PhysicalDeviceIndex].m_Features;
	void* deviceCI_pNext = nullptr;
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (IsActive(m_ActiveInstanceExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) || m_AI.apiVersion >= VK_API_VERSION_1_1)
		deviceCI_pNext = &m_PhysicalDevices.m_PDIs[m_PhysicalDeviceIndex].m_Features2;
#endif

	m_DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	m_DeviceCI.pNext = deviceCI_pNext;
	m_DeviceCI.flags = 0;
	m_DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(m_DeviceQueueCIs.size());
	m_DeviceCI.pQueueCreateInfos = m_DeviceQueueCIs.data();
	m_DeviceCI.enabledLayerCount = static_cast<uint32_t>(m_ActiveDeviceLayers.size());
	m_DeviceCI.ppEnabledLayerNames = m_ActiveDeviceLayers.data();
	m_DeviceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveDeviceExtensions.size());
	m_DeviceCI.ppEnabledExtensionNames = m_ActiveDeviceExtensions.data();
	m_DeviceCI.pEnabledFeatures = deviceCI_pNext ? nullptr : physicalDeviceFeatures;

	MIRU_FATAL(vkCreateDevice(physicalDevice, &m_DeviceCI, nullptr, &m_Device), "ERROR: VULKAN: Failed to create Device");

	//Load Device Extension PFN
	LoadDeviceExtensionPFNs();

	SetResultInfo();

	//Set Names
	//VKSetName<VkInstance>(m_Device, m_Instance, std::string(m_AI.pEngineName) + " - VkInstance");
	VKSetName<VkPhysicalDevice>(m_Device, physicalDevice, "PhysicalDevice: " + std::string(m_PhysicalDevices.m_PDIs[m_PhysicalDeviceIndex].m_Properties.deviceName));
	VKSetName<VkDevice>(m_Device, m_Device, m_CI.deviceDebugName);
	
	//Device Queues
	for (size_t i = 0; i < m_DeviceQueueCIs.size(); i++)
	{
		std::vector<VkQueue>localQueues;
		for (size_t j = 0; j < m_DeviceQueueCIs[i].queueCount; j++)
		{
			VkQueue queue;
			vkGetDeviceQueue(m_Device, static_cast<uint32_t>(i), static_cast<uint32_t>(j), &queue);
			localQueues.push_back(queue);
			
			VkQueueFlagBits flags = static_cast<VkQueueFlagBits>(m_QueueFamilyProperties[i].queueFlags);
			std::string typeStr("");
			if (arc::BitwiseCheck(flags, VK_QUEUE_GRAPHICS_BIT))
				typeStr += "Graphics/";
			if (arc::BitwiseCheck(flags, VK_QUEUE_COMPUTE_BIT))
				typeStr += "Compute/";
			if (arc::BitwiseCheck(flags, VK_QUEUE_TRANSFER_BIT))
				typeStr += "Transfer";
			VKSetName<VkQueue>(m_Device, queue, m_CI.deviceDebugName + ": Queue - " + typeStr);
		}
		m_Queues.push_back(localQueues);
		localQueues.clear();
	}
}

Context::~Context()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (IsActive(m_ActiveInstanceExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugUtilsMessenger, nullptr);

	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

bool Context::IsActive(std::vector<const char*> list, const char* name)
{
	bool found = false;
	for (auto& item : list)
	{
		if (strcmp(name, item) == 0)
		{
			found = true;
			break;
		}
	}
	return found;
}

VkBool32 Context::MessageCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	auto GetMessageSeverityString = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)->std::string
	{
		bool separator = false;

		std::string msg_flags;
		if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
		{
			msg_flags += "VERBOSE";
			separator = true;
		}
		if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
		{
			if (separator) 
				msg_flags += ",";
			msg_flags += "INFO";
			separator = true;
		}
		if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
		{
			if (separator) 
				msg_flags += ",";
			msg_flags += "WARN";
			separator = true;
		}
		if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
		{
			if (separator) 
				msg_flags += ",";
			msg_flags += "ERROR";
		}
		return msg_flags;
	};
	auto GetMessageTypeString = [](VkDebugUtilsMessageTypeFlagBitsEXT messageType)->std::string
	{
		bool separator = false;

		std::string msg_flags;
		if (arc::BitwiseCheck(messageType, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT))
		{
			msg_flags += "GEN";
			separator = true;
		}
		if (arc::BitwiseCheck(messageType, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "SPEC";
			separator = true;
		}
		if (arc::BitwiseCheck(messageType, VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "PERF";
		}
		return msg_flags;
	};

	std::string messageIdName = (pCallbackData->pMessageIdName) ? pCallbackData->pMessageIdName : "";
	std::string messageSeverityStr = GetMessageSeverityString(messageSeverity);
	std::string messageTypeStr = GetMessageTypeString(VkDebugUtilsMessageTypeFlagBitsEXT(messageType));
	int32_t messageIdNumber = pCallbackData->messageIdNumber;
	std::string message = (pCallbackData->pMessage) ? pCallbackData->pMessage : "";

	std::stringstream errorMessage;
	errorMessage << messageIdName << "(" << messageSeverityStr << " / " << messageTypeStr << "): msgNum: " << messageIdNumber << " - " << message;
	std::string errorMessageStr = errorMessage.str();

	if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
	{
		MIRU_ERROR(messageIdNumber, errorMessageStr.c_str());
		ARC_DEBUG_BREAK;
	}
	else if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
	{
		MIRU_WARN(messageIdNumber, errorMessageStr.c_str());
	}
	else if (arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) || arc::BitwiseCheck(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
	{
		MIRU_INFO(messageIdNumber, errorMessageStr.c_str());
	}

	return VK_FALSE;
}

void Context::AddExtensions()
{
	if (m_AI.apiVersion >= VK_API_VERSION_1_0)
	{
		if (m_AI.apiVersion < VK_API_VERSION_1_1)
		{
			m_InstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			//Promoted to Vulkan 1.1
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::TIMELINE_SEMAPHORE))
		{
			m_DeviceExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
			//Required by VK_KHR_timeline_semaphore.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SYNCHRONISATION_2))
		{
			m_DeviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
			//Required by VK_KHR_synchronization2.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::MESH_SHADER))
		{
			m_DeviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
			//Required by VK_EXT_mesh_shader.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::DYNAMIC_RENDERING))
		{
			m_DeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
			//Required by VK_KHR_dynamic_rendering.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::MULTIVIEW))
		{
			m_DeviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
			//Required by VK_KHR_multiview.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER))
		{
			m_DeviceExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SHADER_NATIVE_16_BIT_TYPES))
		{
			m_DeviceExtensions.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
			//Required by VK_KHR_shader_float16_int8.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
			m_DeviceExtensions.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
			//Required by VK_KHR_16bit_storage.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
	}

	if (m_AI.apiVersion >= VK_API_VERSION_1_1)
	{
		//Extensions
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::RAY_TRACING))
		{
			m_DeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

			//Required by VK_KHR_acceleration_structure.
			if (m_AI.apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME); //Promoted to Vulkan 1.2
			if (m_AI.apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME); //Promoted to Vulkan 1.2
			m_DeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

			//Required for VK_KHR_ray_tracing_pipeline.
			if (m_AI.apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME); //Promoted to Vulkan 1.2

			//Required by VK_KHR_spirv_1_4
			if (m_AI.apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME); //Promoted to Vulkan 1.2
		}
	}
}

void Context::SetResultInfo()
{
	m_RI.activeExtensions = ExtensionsBit::NONE;
	//VK_KHR_ray_tracing_pipeline & VK_KHR_acceleration_structure
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) && IsActive(m_ActiveDeviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::RAY_TRACING;
	
	//VK_KHR_timeline_semaphore
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::TIMELINE_SEMAPHORE;

	//VK_EXT_mesh_shader
	if (IsActive(m_ActiveDeviceExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::MESH_SHADER;
	
	//VK_KHR_synchronization2
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::SYNCHRONISATION_2;

	//VK_KHR_dynamic_rendering
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::DYNAMIC_RENDERING;
	
	//VK_KHR_multiview
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::MULTIVIEW;
	
	//VK_EXT_shader_viewport_index_layer
	if (IsActive(m_ActiveDeviceExtensions, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;

	//VK_KHR_shader_float16_int8, VK_KHR_16bit_storage
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)
		&& IsActive(m_ActiveDeviceExtensions, VK_KHR_16BIT_STORAGE_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::SHADER_NATIVE_16_BIT_TYPES;
	
	m_RI.apiVersionMajor = VK_API_VERSION_MAJOR(m_PhysicalDevices.m_PDIs[0].m_Properties.apiVersion);
	m_RI.apiVersionMinor = VK_API_VERSION_MINOR(m_PhysicalDevices.m_PDIs[0].m_Properties.apiVersion);
	m_RI.apiVersionPatch = VK_API_VERSION_PATCH(m_PhysicalDevices.m_PDIs[0].m_Properties.apiVersion);

	m_RI.deviceName = m_PhysicalDevices.m_PDIs[m_PhysicalDeviceIndex].m_Properties.deviceName;
}

#define _STR(str) #str
#define MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(ext) if(IsActive(m_ActiveInstanceExtensions, _STR(VK_##ext))) { LoadPFN_VK_##ext(m_Instance); }
#define MIRU_VULKAN_LOAD_DEVICE_EXTENSION(ext) if(IsActive(m_ActiveDeviceExtensions, _STR(VK_##ext))) { LoadPFN_VK_##ext(m_Device); }

void Context::LoadInstanceExtensionPFNs()
{
	//VK_KHR_get_physical_device_properties2
	MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(KHR_get_physical_device_properties2);

	//VK_EXT_debug_utils
	MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(EXT_debug_utils);
}

void Context::LoadDeviceExtensionPFNs()
{

	//VK_KHR_ray_tracing_pipeline
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_ray_tracing_pipeline);

	//VK_KHR_acceleration_structure
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_acceleration_structure);
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_deferred_host_operations);

	//VK_KHR_buffer_device_address
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_buffer_device_address);

	//VK_KHR_timeline_semaphore
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_timeline_semaphore);

	//VK_EXT_mesh_shader
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(EXT_mesh_shader);

	//VK_KHR_synchronization2
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_synchronization2);

	//VK_KHR_dynamic_rendering
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_dynamic_rendering);
}

Context::PhysicalDevices::PhysicalDevices(const VkInstance& instance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	uint32_t physicalDeviceCount = 0;
	MIRU_FATAL(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
	m_PDIs.resize(physicalDeviceCount);
	std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
	MIRU_FATAL(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, vkPhysicalDevices.data()), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
	for (size_t i = 0; i < vkPhysicalDevices.size(); i++)
	{
		if (vkPhysicalDevices[i] == VK_NULL_HANDLE)
		{
			std::vector<VkPhysicalDevice>::const_iterator it = vkPhysicalDevices.cbegin() + i;
			vkPhysicalDevices.erase(it);
		}
	}
	if (vkPhysicalDevices.empty())
	{
		MIRU_FATAL(true, "ERROR: VULKAN: No valid Vulkan devices are available.");
	}
	for (size_t i = 0; i < m_PDIs.size(); i++) 
		m_PDIs[i].m_PhysicalDevice = vkPhysicalDevices[i]; 
}

void Context::PhysicalDevices::FillOutFeaturesAndProperties(Context* pContext)
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (PhysicalDeviceInfo& pdi : m_PDIs)
	{
		vkGetPhysicalDeviceFeatures(pdi.m_PhysicalDevice, &pdi.m_Features);
		vkGetPhysicalDeviceProperties(pdi.m_PhysicalDevice, &pdi.m_Properties);
		vkGetPhysicalDeviceMemoryProperties(pdi.m_PhysicalDevice, &pdi.m_MemoryProperties);
		uint32_t instanceApiVersion = pContext->m_AI.apiVersion;
		uint32_t deviceApiVersion = pdi.m_Properties.apiVersion;
		
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
		if (IsActive(pContext->m_ActiveInstanceExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) || instanceApiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
		{
			//Features2
			void** nextPropsAddr = nullptr;
			nextPropsAddr = &pdi.m_Features2.pNext;

			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
			{
				pdi.m_RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_RayTracingPipelineFeatures;
				nextPropsAddr = &pdi.m_RayTracingPipelineFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_RAY_QUERY_EXTENSION_NAME))
			{
				pdi.m_RayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_RayQueryFeatures;
				nextPropsAddr = &pdi.m_RayQueryFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
			{
				pdi.m_AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_AccelerationStructureFeatures;
				nextPropsAddr = &pdi.m_AccelerationStructureFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
			{
				pdi.m_BufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
				*nextPropsAddr = &pdi.m_BufferDeviceAddressFeatures;
				nextPropsAddr = &pdi.m_BufferDeviceAddressFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
			{
				pdi.m_TimelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
				*nextPropsAddr = &pdi.m_TimelineSemaphoreFeatures;
				nextPropsAddr = &pdi.m_TimelineSemaphoreFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_3) //Promoted to Vulkan 1.3
			{
				pdi.m_Synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
				*nextPropsAddr = &pdi.m_Synchronization2Features;
				nextPropsAddr = &pdi.m_Synchronization2Features.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
			{
				pdi.m_DeviceMeshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
				*nextPropsAddr = &pdi.m_DeviceMeshShaderFeatures;
				nextPropsAddr = &pdi.m_DeviceMeshShaderFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_3) //Promoted to Vulkan 1.3
			{
				pdi.m_DynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
				*nextPropsAddr = &pdi.m_DynamicRenderingFeatures;
				nextPropsAddr = &pdi.m_DynamicRenderingFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
			{
				pdi.m_MultivewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
				*nextPropsAddr = &pdi.m_MultivewFeatures;
				nextPropsAddr = &pdi.m_MultivewFeatures.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
			{
				pdi.m_ShaderFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_ShaderFloat16Int8Features;
				nextPropsAddr = &pdi.m_ShaderFloat16Int8Features.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_16BIT_STORAGE_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
			{
				pdi.m_16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_16BitStorageFeatures;
				nextPropsAddr = &pdi.m_16BitStorageFeatures.pNext;
			}
			if (deviceApiVersion >= VK_API_VERSION_1_1)
			{
				pdi.m_Vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
				*nextPropsAddr = &pdi.m_Vulkan11Features;
				nextPropsAddr = &pdi.m_Vulkan11Features.pNext;
			}
			if (deviceApiVersion >= VK_API_VERSION_1_2)
			{
				pdi.m_Vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				*nextPropsAddr = &pdi.m_Vulkan12Features;
				nextPropsAddr = &pdi.m_Vulkan12Features.pNext;
			}
			if (deviceApiVersion >= VK_API_VERSION_1_3)
			{
				pdi.m_Vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
				*nextPropsAddr = &pdi.m_Vulkan13Features;
				nextPropsAddr = &pdi.m_Vulkan13Features.pNext;
			}
			pdi.m_Features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			vkGetPhysicalDeviceFeatures2(pdi.m_PhysicalDevice, &pdi.m_Features2);
		
			//Properties2
			nextPropsAddr = nullptr;
			nextPropsAddr = &pdi.m_Properties2.pNext;
			
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
			{
				pdi.m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
				*nextPropsAddr = &pdi.m_RayTracingPipelineProperties;
				nextPropsAddr = &pdi.m_RayTracingPipelineProperties.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
			{
				pdi.m_AccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
				*nextPropsAddr = &pdi.m_AccelerationStructureProperties;
				nextPropsAddr = &pdi.m_AccelerationStructureProperties.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) || deviceApiVersion >= VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
			{
				pdi.m_TimelineSemaphoreProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES;
				*nextPropsAddr = &pdi.m_TimelineSemaphoreProperties;
				nextPropsAddr = &pdi.m_TimelineSemaphoreProperties.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
			{
				pdi.m_DeviceMeshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
				*nextPropsAddr = &pdi.m_DeviceMeshShaderProperties;
				nextPropsAddr = &pdi.m_DeviceMeshShaderProperties.pNext;
			}
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME) || deviceApiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
			{
				pdi.m_MultivewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
				*nextPropsAddr = &pdi.m_MultivewProperties;
				nextPropsAddr = &pdi.m_MultivewProperties.pNext;
			}
			if (deviceApiVersion >= VK_API_VERSION_1_1)
			{
				pdi.m_Vulkan11Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
				*nextPropsAddr = &pdi.m_Vulkan11Properties;
				nextPropsAddr = &pdi.m_Vulkan11Properties.pNext;
			}
			if (deviceApiVersion >= VK_API_VERSION_1_2)
			{
				pdi.m_Vulkan12Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
				*nextPropsAddr = &pdi.m_Vulkan12Properties;
				nextPropsAddr = &pdi.m_Vulkan12Properties.pNext;
			}
			if (deviceApiVersion >= VK_API_VERSION_1_3)
			{
				pdi.m_Vulkan13Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
				*nextPropsAddr = &pdi.m_Vulkan13Properties;
				nextPropsAddr = &pdi.m_Vulkan13Properties.pNext;
			}
			pdi.m_Properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			vkGetPhysicalDeviceProperties2(pdi.m_PhysicalDevice, &pdi.m_Properties2);

			//VkPhysicalDeviceFragmentShadingRateFeaturesKHR is not supported at the moment.
			//This feature therefore must be disabled if mesh shader is requested.
			pdi.m_DeviceMeshShaderFeatures.primitiveFragmentShadingRateMeshShader = false;
		}
#endif
	}
}