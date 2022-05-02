#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKContext.h"

using namespace miru;
using namespace vulkan;

Context::Context(Context::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//Instance
	uint32_t apiVersion = 0;
	vkEnumerateInstanceVersion(&apiVersion);
	

	#if defined(VK_USE_PLATFORM_WIN32_KHR)
	const char* engineName = "MIRU - x64";
	#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	engineName = "MIRU - ARM64";
	#endif

	m_AI.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	m_AI.pNext = nullptr;
	m_AI.pApplicationName = m_CI.applicationName.c_str();
	m_AI.applicationVersion = 1;
	m_AI.pEngineName = engineName;
	m_AI.engineVersion = 1;
	m_AI.apiVersion = apiVersion;

	//Add additional instance/device layers/extensions
	#if defined(VK_VERSION_1_0)
	if (apiVersion >= VK_API_VERSION_1_0)
	{
		//Debug
		#if defined(_DEBUG)
		m_CI.debugValidationLayers = true;
		#endif
		if (m_CI.debugValidationLayers)
		{
			m_InstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
			m_DeviceLayers.push_back("VK_LAYER_KHRONOS_validation");
		}
		if (crossplatform::GraphicsAPI::IsSetNameAllowed())
			m_InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		//Surface and Swapchain
		m_InstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		m_DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		m_InstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_ANDROIOD_KHR)
		instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
		#endif

		//Extensions
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::DYNAMIC_RENDERING))
		{
			#if defined(VK_KHR_dynamic_rendering)
			m_DeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
			//Required by VK_KHR_dynamic_rendering
			if (apiVersion < VK_API_VERSION_1_1)
				m_InstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); //Promoted to Vulkan 1.1
			#endif
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::MULTIVIEW))
		{
			#if defined(VK_KHR_multiview)
			m_DeviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
			//Required by VK_KHR_multiview
			if (apiVersion < VK_API_VERSION_1_1)
				m_InstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); //Promoted to Vulkan 1.1
			#endif
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER))
		{
			#if defined(VK_EXT_shader_viewport_index_layer)
			m_DeviceExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
			#endif
		}
	}
	#endif
	#if defined(VK_VERSION_1_1)
	if (apiVersion >= VK_API_VERSION_1_1)
	{
		//Extensions
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::RAY_TRACING))
		{
			#if defined(VK_KHR_ray_tracing_pipeline) && defined(VK_KHR_acceleration_structure)
			//Required for ExtensionsBit::RAY_TRACING
			m_DeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			m_DeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

			//Required by VK_KHR_acceleration_structure
			if (apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME); //Promoted to Vulkan 1.2
			if (apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME); //Promoted to Vulkan 1.2
			m_DeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

			//Required for VK_KHR_ray_tracing_pipeline
			if (apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME); //Promoted to Vulkan 1.2

			//Required by VK_KHR_spirv_1_4
			if (apiVersion < VK_API_VERSION_1_2)
				m_DeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME); //Promoted to Vulkan 1.2
			#endif
		}
	}
	#endif

	uint32_t instanceLayerCount = 0;
	MIRU_ASSERT(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
	std::vector<VkLayerProperties> instanceLayerProperties;
	instanceLayerProperties.resize(instanceLayerCount);
	MIRU_ASSERT(vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceLayerProperties.");
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
	MIRU_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
	std::vector<VkExtensionProperties> instanceExtensionProperties;
	instanceExtensionProperties.resize(instanceExtensionCount);
	MIRU_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate InstanceExtensionProperties.");
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

	MIRU_ASSERT(vkCreateInstance(&m_InstanceCI, nullptr, &m_Instance), "ERROR: VULKAN: Failed to create Instance.");

	//PhysicalDevice
	m_PhysicalDevices = PhysicalDevices(m_Instance, apiVersion);
	VkPhysicalDevice physicalDevice = m_PhysicalDevices.m_PDIs[0].m_PhysicalDevice; //We only use the first PhysicalDevice

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
	MIRU_ASSERT(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, nullptr), "ERROR: VULKAN: Failed to enumerate DeviceLayerProperties.");
	std::vector<VkLayerProperties> deviceLayerProperties;
	deviceLayerProperties.resize(deviceLayerCount);
	MIRU_ASSERT(vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerCount, deviceLayerProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceLayerProperties.");
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
	MIRU_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, 0), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	std::vector<VkExtensionProperties> deviceExtensionProperties;
	deviceExtensionProperties.resize(deviceExtensionCount);
	MIRU_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, deviceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
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
	VkPhysicalDeviceFeatures physicalDeviceFeatures = m_PhysicalDevices.m_PDIs[0].m_Features;
	m_PhysicalDevices.FillOutFeaturesAndProperties(this);
	void* deviceCI_pNext = nullptr;
	#if defined(VK_KHR_get_physical_device_properties2)
	if (IsActive(m_ActiveInstanceExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) || m_AI.apiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
		deviceCI_pNext = &m_PhysicalDevices.m_PDIs[0].m_Features2;
	#endif

	m_RI.activeExtensions = ExtensionsBit::NONE;
	#if defined(VK_KHR_ray_tracing_pipeline) && defined(VK_KHR_acceleration_structure)
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
		&& IsActive(m_ActiveDeviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
	{
		m_RI.activeExtensions |= ExtensionsBit::RAY_TRACING;
	}
	#endif
	#if defined(VK_KHR_dynamic_rendering)
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME))
	{
		m_RI.activeExtensions |= ExtensionsBit::DYNAMIC_RENDERING;
	}
	#endif
	#if defined(VK_KHR_multiview)
	if (IsActive(m_ActiveDeviceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME))
	{
		m_RI.activeExtensions |= ExtensionsBit::MULTIVIEW;
	}
	#endif
	#if defined(VK_EXT_shader_viewport_index_layer)
	if (IsActive(m_ActiveDeviceExtensions, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME))
	{
		m_RI.activeExtensions |= ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;
	}
	#endif
	m_RI.apiVersionMajor = VK_API_VERSION_MAJOR(m_PhysicalDevices.m_PDIs[0].m_Properties.apiVersion);
	m_RI.apiVersionMinor = VK_API_VERSION_MINOR(m_PhysicalDevices.m_PDIs[0].m_Properties.apiVersion);
	m_RI.apiVersionPatch = VK_API_VERSION_PATCH(m_PhysicalDevices.m_PDIs[0].m_Properties.apiVersion);

	m_DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	m_DeviceCI.pNext = deviceCI_pNext;
	m_DeviceCI.flags = 0;
	m_DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(m_DeviceQueueCIs.size());
	m_DeviceCI.pQueueCreateInfos = m_DeviceQueueCIs.data();
	m_DeviceCI.enabledLayerCount = static_cast<uint32_t>(m_ActiveDeviceLayers.size());
	m_DeviceCI.ppEnabledLayerNames = m_ActiveDeviceLayers.data();
	m_DeviceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveDeviceExtensions.size());
	m_DeviceCI.ppEnabledExtensionNames = m_ActiveDeviceExtensions.data();
	m_DeviceCI.pEnabledFeatures = deviceCI_pNext ? nullptr : &physicalDeviceFeatures;;

	MIRU_ASSERT(vkCreateDevice(physicalDevice, &m_DeviceCI, nullptr, &m_Device), "ERROR: VULKAN: Failed to create Device");

	//Load Extension PFN
	#define _STR(str) #str
	#define MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(ext) if(IsActive(m_ActiveInstanceExtensions, _STR(VK_##ext))) { LoadPFN_VK_##ext(m_Instance); }
	#define MIRU_VULKAN_LOAD_DEVICE_EXTENSION(ext) if(IsActive(m_ActiveDeviceExtensions, _STR(VK_##ext))) { LoadPFN_VK_##ext(m_Device); }

#if defined(VK_EXT_debug_utils)
	MIRU_VULKAN_LOAD_INSTANCE_EXTENSION(EXT_debug_utils);
#endif

#if defined(VK_KHR_buffer_device_address)
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_buffer_device_address);
#endif

#if defined(VK_KHR_ray_tracing_pipeline)
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_ray_tracing_pipeline);
#endif

#if defined(VK_KHR_acceleration_structure)
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_acceleration_structure);
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_deferred_host_operations);
#endif

#if defined(VK_KHR_dynamic_rendering)
	MIRU_VULKAN_LOAD_DEVICE_EXTENSION(KHR_dynamic_rendering);
#endif

	//Set Names
	//VKSetName<VkInstance>(m_Device, m_Instance, std::string(m_AI.pEngineName) + " - VkInstance");
	VKSetName<VkPhysicalDevice>(m_Device, m_PhysicalDevices.m_PDIs[0].m_PhysicalDevice, "PhysicalDevice: " + std::string(m_PhysicalDevices.m_PDIs[0].m_Properties.deviceName));
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

Context::PhysicalDevices::PhysicalDevices(const VkInstance& instance, uint32_t apiVersion)
{
	MIRU_CPU_PROFILE_FUNCTION();

	uint32_t physicalDeviceCount = 0;
	MIRU_ASSERT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
	m_PDIs.resize(physicalDeviceCount);
	std::vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount);
	MIRU_ASSERT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, vkPhysicalDevices.data()), "ERROR: VULKAN: Failed to enumerate PhysicalDevices.");
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
		MIRU_ASSERT(true, "ERROR: VULKAN: No valid Vulkan devices are available.");
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
		
		#if defined(VK_KHR_get_physical_device_properties2)
		if (IsActive(pContext->m_ActiveInstanceExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) || pContext->m_AI.apiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
		{
			//Features2
			void** nextPropsAddr = nullptr;
			nextPropsAddr = &pdi.m_Features2.pNext;

			#if defined(VK_KHR_multiview)
			if (IsActive(pContext->m_ActiveInstanceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME) || pContext->m_AI.apiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
			{
				pdi.m_MultivewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_MultivewFeatures;
				nextPropsAddr = &pdi.m_MultivewFeatures.pNext;
			}
			#endif

			#if defined(VK_KHR_buffer_device_address)
			if (IsActive(pContext->m_ActiveInstanceExtensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) || pContext->m_AI.apiVersion >= VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
			{
				pdi.m_BufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_BufferDeviceAddressFeatures;
				nextPropsAddr = &pdi.m_BufferDeviceAddressFeatures.pNext;
			}
			#endif

			#if defined(VK_KHR_ray_tracing_pipeline)
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
			{
				pdi.m_RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_RayTracingPipelineFeatures;
				nextPropsAddr = &pdi.m_RayTracingPipelineFeatures.pNext;
			}
			#endif

			#if defined(VK_KHR_acceleration_structure)
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
			{
				pdi.m_AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_AccelerationStructureFeatures;
				nextPropsAddr = &pdi.m_AccelerationStructureFeatures.pNext;
			}
			#endif

			#if defined(VK_KHR_dynamic_rendering)
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME))
			{
				pdi.m_DynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
				*nextPropsAddr = &pdi.m_DynamicRenderingFeatures;
				nextPropsAddr = &pdi.m_DynamicRenderingFeatures.pNext;
			}
			#endif

			pdi.m_Features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			vkGetPhysicalDeviceFeatures2(pdi.m_PhysicalDevice, &pdi.m_Features2);
		

			//Properties2
			nextPropsAddr = nullptr;
			nextPropsAddr = &pdi.m_Properties2.pNext;
			
			#if defined(VK_KHR_multiview)
			if (IsActive(pContext->m_ActiveInstanceExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME) || pContext->m_AI.apiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
			{
				pdi.m_MultivewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHR;
				*nextPropsAddr = &pdi.m_MultivewProperties;
				nextPropsAddr = &pdi.m_MultivewProperties.pNext;
			}
			#endif

			#if defined(VK_KHR_ray_tracing_pipeline)
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
			{
				pdi.m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
				*nextPropsAddr = &pdi.m_RayTracingPipelineProperties;
				nextPropsAddr = &pdi.m_RayTracingPipelineProperties.pNext;
			}
			#endif

			#if defined(VK_KHR_acceleration_structure)
			if (IsActive(pContext->m_ActiveDeviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
			{
				pdi.m_AccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
				*nextPropsAddr = &pdi.m_AccelerationStructureProperties;
				nextPropsAddr = &pdi.m_AccelerationStructureProperties.pNext;
			}
			#endif

			pdi.m_Properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			vkGetPhysicalDeviceProperties2(pdi.m_PhysicalDevice, &pdi.m_Properties2);
		}
#endif
	}
}

#endif