#include "VKDevice.h"
#include "VKPhysicalDevice.h"
#include "VKInstance.h"

using namespace miru;
using namespace vulkan;

Device::Device(CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	InstanceRef instance = ref_cast<Instance>(GetInstance());
	PhysicalDeviceRef physicalDeviceRef = ref_cast<PhysicalDevice>(m_CI.physicalDevice);
	VkPhysicalDevice physicalDevice = reinterpret_cast<VkPhysicalDevice>(physicalDeviceRef->m_CI.nativeHandle);
	
	//Add additional device layers/extensions
	{
		//Debug
		if (m_CI.debugValidationLayers)
		{
			m_Layers.push_back("VK_LAYER_KHRONOS_validation");
			m_Layers.push_back("VK_LAYER_KHRONOS_synchronization2");
		}

		//Surface and Swapchain
		m_Extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		//Device
		AddExtensions();

		//OpenXR Data
		Instance::OpenXRVulkanData* openXRVulkanData = reinterpret_cast<Instance::OpenXRVulkanData*>(instance->GetCreateInfo().pNext);
		if (!(openXRVulkanData && openXRVulkanData->type == Instance::CreateInfoExtensionStructureTypes::OPENXR_VULKAN_DATA))
			openXRVulkanData = nullptr;

		//OpenXR Extensions
		if (openXRVulkanData)
		{
			for (const auto& deviceExtension : openXRVulkanData->deviceExtensions)
				m_Extensions.push_back(deviceExtension);
		}
	}

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
	for (auto& requestLayer : m_Layers)
	{
		for (auto& layerProperty : deviceLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveLayers.push_back(requestLayer.c_str()); break;
		}
	}
	uint32_t deviceExtensionCount = 0;
	MIRU_FATAL(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, 0), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	std::vector<VkExtensionProperties> deviceExtensionProperties;
	deviceExtensionProperties.resize(deviceExtensionCount);
	MIRU_FATAL(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &deviceExtensionCount, deviceExtensionProperties.data()), "ERROR: VULKAN: Failed to enumerate DeviceExtensionProperties.");
	for (auto& requestExtension : m_Extensions)
	{
		for (auto& extensionProperty : deviceExtensionProperties)
		{
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				m_ActiveExtensions.push_back(requestExtension.c_str()); break;
		}
	}

	//PhysicalDevice Features
	VkPhysicalDeviceFeatures* physicalDeviceFeatures = &physicalDeviceRef->m_Features;
	void* deviceCI_pNext = nullptr;
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (Instance::IsActive(instance->m_ActiveExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) || physicalDeviceRef->m_Properties.apiVersion >= VK_API_VERSION_1_1)
	{
		m_FeatureAndProperties = FeaturesAndProperties(this);
		deviceCI_pNext = &m_FeatureAndProperties.m_Features2;
	}
#endif

	m_DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	m_DeviceCI.pNext = deviceCI_pNext;
	m_DeviceCI.flags = 0;
	m_DeviceCI.queueCreateInfoCount = static_cast<uint32_t>(m_DeviceQueueCIs.size());
	m_DeviceCI.pQueueCreateInfos = m_DeviceQueueCIs.data();
	m_DeviceCI.enabledLayerCount = static_cast<uint32_t>(m_ActiveLayers.size());
	m_DeviceCI.ppEnabledLayerNames = m_ActiveLayers.data();
	m_DeviceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveExtensions.size());
	m_DeviceCI.ppEnabledExtensionNames = m_ActiveExtensions.data();
	m_DeviceCI.pEnabledFeatures = deviceCI_pNext ? nullptr : physicalDeviceFeatures;

	MIRU_FATAL(vkCreateDevice(physicalDevice, &m_DeviceCI, nullptr, &m_Device), "ERROR: VULKAN: Failed to create Device");

	//Load Device Extension PFN
	LoadDeviceExtensionPFNs();

	SetResultInfo();

	//Set Name
	VKSetName<VkInstance>(m_Device, instance->m_Instance, "Instance: " + std::string(instance->m_AI.pEngineName));
	VKSetName<VkPhysicalDevice>(m_Device, physicalDevice, "PhysicalDevice: " + std::string(physicalDeviceRef->m_Properties.deviceName));
	VKSetName<VkDevice>(m_Device, m_Device, m_CI.debugName);

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
			VKSetName<VkQueue>(m_Device, queue, m_CI.debugName + ": Queue - " + typeStr);
		}
		m_Queues.push_back(localQueues);
		localQueues.clear();
	}
}

Device::~Device()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyDevice(m_Device, nullptr);
}

void Device::AddExtensions()
{
	MIRU_CPU_PROFILE_FUNCTION();

	PhysicalDeviceRef physicalDeviceRef = ref_cast<vulkan::PhysicalDevice>(m_CI.physicalDevice);
	const uint32_t& apiVersion = physicalDeviceRef->m_Properties.apiVersion;

	if (apiVersion >= VK_API_VERSION_1_0)
	{
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::TIMELINE_SEMAPHORE))
		{
			m_Extensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
			//Required by VK_KHR_timeline_semaphore.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SYNCHRONISATION_2))
		{
			m_Extensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
			//Required by VK_KHR_synchronization2.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::MESH_SHADER))
		{
			m_Extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
			//Required by VK_EXT_mesh_shader.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::DYNAMIC_RENDERING))
		{
			m_Extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
			//Required by VK_KHR_dynamic_rendering.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::MULTIVIEW))
		{
			m_Extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
			//Required by VK_KHR_multiview.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER))
		{
			m_Extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
		}
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::SHADER_NATIVE_16_BIT_TYPES))
		{
			m_Extensions.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
			//Required by VK_KHR_shader_float16_int8.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
			m_Extensions.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
			//Required by VK_KHR_16bit_storage.
			//VK_KHR_get_physical_device_properties2 already loaded, if needed.
		}
	}

	if (apiVersion >= VK_API_VERSION_1_1)
	{
		//Extensions
		if (arc::BitwiseCheck(m_CI.extensions, ExtensionsBit::RAY_TRACING))
		{
			m_Extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
			m_Extensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
			m_Extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

			//Required by VK_KHR_acceleration_structure.
			if (apiVersion < VK_API_VERSION_1_2)
				m_Extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME); //Promoted to Vulkan 1.2
			if (apiVersion < VK_API_VERSION_1_2)
				m_Extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME); //Promoted to Vulkan 1.2
			m_Extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

			//Required for VK_KHR_ray_tracing_pipeline.
			if (apiVersion < VK_API_VERSION_1_2)
				m_Extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME); //Promoted to Vulkan 1.2

			//Required by VK_KHR_spirv_1_4
			if (apiVersion < VK_API_VERSION_1_2)
				m_Extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME); //Promoted to Vulkan 1.2
		}
	}
}

void Device::SetResultInfo()
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_RI.activeExtensions = ExtensionsBit::NONE;
	//VK_KHR_ray_tracing_pipeline & VK_KHR_acceleration_structure
	if (Instance::IsActive(m_ActiveExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) && Instance::IsActive(m_ActiveExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::RAY_TRACING;

	//VK_KHR_timeline_semaphore
	if (Instance::IsActive(m_ActiveExtensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::TIMELINE_SEMAPHORE;

	//VK_EXT_mesh_shader
	if (Instance::IsActive(m_ActiveExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::MESH_SHADER;

	//VK_KHR_synchronization2
	if (Instance::IsActive(m_ActiveExtensions, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::SYNCHRONISATION_2;

	//VK_KHR_dynamic_rendering
	if (Instance::IsActive(m_ActiveExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::DYNAMIC_RENDERING;

	//VK_KHR_multiview
	if (Instance::IsActive(m_ActiveExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::MULTIVIEW;

	//VK_EXT_shader_viewport_index_layer
	if (Instance::IsActive(m_ActiveExtensions, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;

	//VK_KHR_shader_float16_int8, VK_KHR_16bit_storage
	if (Instance::IsActive(m_ActiveExtensions, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)
		&& Instance::IsActive(m_ActiveExtensions, VK_KHR_16BIT_STORAGE_EXTENSION_NAME))
		m_RI.activeExtensions |= ExtensionsBit::SHADER_NATIVE_16_BIT_TYPES;

	PhysicalDeviceRef physicalDevice = ref_cast<PhysicalDevice>(m_CI.physicalDevice);
	m_RI.apiVersionMajor = VK_API_VERSION_MAJOR(physicalDevice->m_Properties.apiVersion);
	m_RI.apiVersionMinor = VK_API_VERSION_MINOR(physicalDevice->m_Properties.apiVersion);
	m_RI.apiVersionPatch = VK_API_VERSION_PATCH(physicalDevice->m_Properties.apiVersion);

	m_RI.deviceName = physicalDevice->m_Properties.deviceName;
}

#define _STR(str) #str
#define MIRU_VULKAN_LOAD_DEVICE_EXTENSION(ext) if(Instance::IsActive(m_ActiveExtensions, _STR(VK_##ext))) { LoadPFN_VK_##ext(m_Device); }

void Device::LoadDeviceExtensionPFNs()
{
	MIRU_CPU_PROFILE_FUNCTION();

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

Device::FeaturesAndProperties::FeaturesAndProperties(const Device* device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	PhysicalDeviceRef physicalDevice = ref_cast<PhysicalDevice>(device->GetPhysicalDevice());
	InstanceRef instance = ref_cast<Instance>(device->GetInstance());

	uint32_t instanceApiVersion = instance->m_AI.apiVersion;
	uint32_t deviceApiVersion = physicalDevice->m_Properties.apiVersion;

	#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (Instance::IsActive(instance->m_ActiveExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) || instanceApiVersion >= VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
	{
		//Features2
		void** nextPropsAddr = nullptr;
		nextPropsAddr = &m_Features2.pNext;

		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
		{
			m_RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			*nextPropsAddr = &m_RayTracingPipelineFeatures;
			nextPropsAddr = &m_RayTracingPipelineFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_RAY_QUERY_EXTENSION_NAME))
		{
			m_RayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
			*nextPropsAddr = &m_RayQueryFeatures;
			nextPropsAddr = &m_RayQueryFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
		{
			m_AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			*nextPropsAddr = &m_AccelerationStructureFeatures;
			nextPropsAddr = &m_AccelerationStructureFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
		{
			m_BufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			*nextPropsAddr = &m_BufferDeviceAddressFeatures;
			nextPropsAddr = &m_BufferDeviceAddressFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
		{
			m_TimelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
			*nextPropsAddr = &m_TimelineSemaphoreFeatures;
			nextPropsAddr = &m_TimelineSemaphoreFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_3) //Promoted to Vulkan 1.3
		{
			m_Synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
			*nextPropsAddr = &m_Synchronization2Features;
			nextPropsAddr = &m_Synchronization2Features.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
		{
			m_DeviceMeshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
			*nextPropsAddr = &m_DeviceMeshShaderFeatures;
			nextPropsAddr = &m_DeviceMeshShaderFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_3) //Promoted to Vulkan 1.3
		{
			m_DynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
			*nextPropsAddr = &m_DynamicRenderingFeatures;
			nextPropsAddr = &m_DynamicRenderingFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
		{
			m_MultivewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
			*nextPropsAddr = &m_MultivewFeatures;
			nextPropsAddr = &m_MultivewFeatures.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
		{
			m_ShaderFloat16Int8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
			*nextPropsAddr = &m_ShaderFloat16Int8Features;
			nextPropsAddr = &m_ShaderFloat16Int8Features.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_16BIT_STORAGE_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
		{
			m_16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
			*nextPropsAddr = &m_16BitStorageFeatures;
			nextPropsAddr = &m_16BitStorageFeatures.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_1)
		{
			m_Vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
			*nextPropsAddr = &m_Vulkan11Features;
			nextPropsAddr = &m_Vulkan11Features.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_2)
		{
			m_Vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			*nextPropsAddr = &m_Vulkan12Features;
			nextPropsAddr = &m_Vulkan12Features.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_3)
		{
			m_Vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			*nextPropsAddr = &m_Vulkan13Features;
			nextPropsAddr = &m_Vulkan13Features.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_4)
		{
			m_Vulkan14Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
			*nextPropsAddr = &m_Vulkan14Features;
			nextPropsAddr = &m_Vulkan14Features.pNext;
		}

		m_Features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(physicalDevice->m_PhysicalDevice, &m_Features2);

		//Properties2
		nextPropsAddr = nullptr;
		nextPropsAddr = &m_Properties2.pNext;

		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
		{
			m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
			*nextPropsAddr = &m_RayTracingPipelineProperties;
			nextPropsAddr = &m_RayTracingPipelineProperties.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
		{
			m_AccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
			*nextPropsAddr = &m_AccelerationStructureProperties;
			nextPropsAddr = &m_AccelerationStructureProperties.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_2) //Promoted to Vulkan 1.2
		{
			m_TimelineSemaphoreProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES;
			*nextPropsAddr = &m_TimelineSemaphoreProperties;
			nextPropsAddr = &m_TimelineSemaphoreProperties.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
		{
			m_DeviceMeshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
			*nextPropsAddr = &m_DeviceMeshShaderProperties;
			nextPropsAddr = &m_DeviceMeshShaderProperties.pNext;
		}
		if (Instance::IsActive(device->m_ActiveExtensions, VK_KHR_MULTIVIEW_EXTENSION_NAME) && deviceApiVersion < VK_API_VERSION_1_1) //Promoted to Vulkan 1.1
		{
			m_MultivewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
			*nextPropsAddr = &m_MultivewProperties;
			nextPropsAddr = &m_MultivewProperties.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_1)
		{
			m_Vulkan11Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
			*nextPropsAddr = &m_Vulkan11Properties;
			nextPropsAddr = &m_Vulkan11Properties.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_2)
		{
			m_Vulkan12Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
			*nextPropsAddr = &m_Vulkan12Properties;
			nextPropsAddr = &m_Vulkan12Properties.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_3)
		{
			m_Vulkan13Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
			*nextPropsAddr = &m_Vulkan13Properties;
			nextPropsAddr = &m_Vulkan13Properties.pNext;
		}
		if (deviceApiVersion >= VK_API_VERSION_1_4)
		{
			m_Vulkan14Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;
			*nextPropsAddr = &m_Vulkan14Properties;
			nextPropsAddr = &m_Vulkan14Properties.pNext;
		}

		m_Properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		vkGetPhysicalDeviceProperties2(physicalDevice->m_PhysicalDevice, &m_Properties2);

		//VkPhysicalDeviceFragmentShadingRateFeaturesKHR is not supported at the moment.
		//This feature therefore must be disabled if mesh shader is requested.
		m_DeviceMeshShaderFeatures.primitiveFragmentShadingRateMeshShader = false;
	}
	#endif
}