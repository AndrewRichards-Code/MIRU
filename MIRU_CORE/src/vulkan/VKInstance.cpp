#include "VkInstance.h"
#include "VkPhysicalDevice.h"

using namespace miru;
using namespace vulkan;

Instance::Instance(Instance::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//Instance
	uint32_t apiVersion = VK_API_VERSION_1_0;
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
	if (vkEnumerateInstanceVersion != nullptr)
		vkEnumerateInstanceVersion(&apiVersion);

	//OpenXR Data
	OpenXRVulkanData* openXRVulkanData = reinterpret_cast<OpenXRVulkanData*>(m_CI.pNext);
	if (!(openXRVulkanData && openXRVulkanData->type == CreateInfoExtensionStructureTypes::OPENXR_VULKAN_DATA))
		openXRVulkanData = nullptr;

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
	m_AI.engineVersion = 2;
	m_AI.apiVersion = apiVersion;

	//Add additional instance/device layers/extensions
	{
		//Debug
		if (m_CI.debugValidationLayers)
		{
			m_Layers.push_back("VK_LAYER_KHRONOS_validation");
			m_Layers.push_back("VK_LAYER_KHRONOS_synchronization2");
		}
		if (base::GraphicsAPI::IsSetNameAllowed())
			m_Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		
		//Surface and Swapchain
		m_Extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		m_Extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		m_Extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
		#endif

		//Displays
		m_Extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
		m_Extensions.push_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
		
		//Extensions
		AddExtensions();

		//OpenXR Extension
		if (openXRVulkanData)
		{
			for (const auto& instanceExtension : openXRVulkanData->instanceExtensions)
				m_Extensions.push_back(instanceExtension);
		}
	}

	uint32_t instanceLayerCount = 0;
	MIRU_FATAL(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	std::vector<VkLayerProperties> instanceLayerProperties;
	instanceLayerProperties.resize(instanceLayerCount);
	MIRU_FATAL(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	for (auto& requestLayer : m_Layers)
	{
		for (auto& layerProperty : instanceLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveLayers.push_back(requestLayer.c_str()); break;
		}
	}
	uint32_t instanceExtensionCount = 0;
	MIRU_FATAL(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	std::vector<VkExtensionProperties> instanceExtensionProperties;
	instanceExtensionProperties.resize(instanceExtensionCount);
	MIRU_FATAL(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	for (auto& requestExtension : m_Extensions)
	{
		for (auto& extensionProperty : instanceExtensionProperties)
		{
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				m_ActiveExtensions.push_back(requestExtension.c_str()); break;
		}
	}

	m_InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	m_InstanceCI.pNext = nullptr;
	m_InstanceCI.flags = 0;
	m_InstanceCI.pApplicationInfo = &m_AI;
	m_InstanceCI.enabledLayerCount = static_cast<uint32_t>(m_ActiveLayers.size());
	m_InstanceCI.ppEnabledLayerNames = m_ActiveLayers.data();
	m_InstanceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveExtensions.size());
	m_InstanceCI.ppEnabledExtensionNames = m_ActiveExtensions.data();

	MIRU_FATAL(vkCreateInstance(&m_InstanceCI, nullptr, &m_Instance), "ERROR: VULKAN: Failed to create Instance.");

	//Load Instance Extension PFN
	LoadInstanceExtensionPFNs();

	//Debug Messenger Callback
	if (IsActive(m_ActiveExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
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

	SetResultInfo();
}

Instance::~Instance()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (IsActive(m_ActiveExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugUtilsMessenger, nullptr);

	vkDestroyInstance(m_Instance, nullptr);
}

base::PhysicalDeviceRefs Instance::GetPhysicalDevices()
{
	MIRU_CPU_PROFILE_FUNCTION();

	uint32_t physicalDeviceCount = 0;
	MIRU_FATAL(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
	std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
	MIRU_FATAL(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, vkPhysicalDevices.data()), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
	
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

	base::PhysicalDeviceRefs physicalDevices;
	for (const VkPhysicalDevice& vkPhysicalDevice : vkPhysicalDevices)
	{
		base::PhysicalDevice::CreateInfo physicalDeviceCI;
		physicalDeviceCI.instance = InstanceRef(this);
		physicalDeviceCI.nativeHandle = vkPhysicalDevice;
		physicalDevices.push_back(base::PhysicalDevice::Create(&physicalDeviceCI));
	}

	return physicalDevices;
}

bool Instance::IsActive(std::vector<const char*> list, const char* name)
{
	MIRU_CPU_PROFILE_FUNCTION();

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

VkBool32 Instance::MessageCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
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

void Instance::AddExtensions()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_AI.apiVersion >= VK_API_VERSION_1_0)
	{
		if (m_AI.apiVersion < VK_API_VERSION_1_1)
		{
			m_Extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
			//Promoted to Vulkan 1.1
		}
	}
}

void Instance::SetResultInfo()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_RI.apiVersionMajor = VK_API_VERSION_MAJOR(m_AI.apiVersion);
	m_RI.apiVersionMinor = VK_API_VERSION_MINOR(m_AI.apiVersion);
	m_RI.apiVersionPatch = VK_API_VERSION_PATCH(m_AI.apiVersion);
}

#define _STR(str) #str
#define MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(ext) if(Instance::IsActive(m_ActiveExtensions, _STR(VK_##ext))) { LoadPFN_VK_##ext(m_Instance); }

void Instance::LoadInstanceExtensionPFNs()
{
	MIRU_CPU_PROFILE_FUNCTION();

	//VK_KHR_get_physical_device_properties2
	MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(KHR_get_physical_device_properties2);

	//VK_EXT_debug_utils
	MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(EXT_debug_utils);
}