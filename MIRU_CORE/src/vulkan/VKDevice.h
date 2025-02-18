#pragma once

#include "base/Device.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class Device final : public base::Device
	{
		//enum/structs
	public:
		struct FeaturesAndProperties
		{
			VkPhysicalDeviceFeatures2							m_Features2 = {};						//VK_KHR_get_physical_device_properties2
			VkPhysicalDeviceProperties2							m_Properties2 = {};						//VK_KHR_get_physical_device_properties2
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR		m_RayTracingPipelineFeatures = {};		//VK_KHR_ray_tracing_pipeline
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR		m_RayTracingPipelineProperties = {};	//VK_KHR_ray_tracing_pipeline
			VkPhysicalDeviceRayQueryFeaturesKHR					m_RayQueryFeatures = {};				//VK_KHR_ray_query
			VkPhysicalDeviceAccelerationStructureFeaturesKHR	m_AccelerationStructureFeatures = {};	//VK_KHR_acceleration_structure
			VkPhysicalDeviceAccelerationStructurePropertiesKHR	m_AccelerationStructureProperties = {};	//VK_KHR_acceleration_structure
			VkPhysicalDeviceBufferDeviceAddressFeatures			m_BufferDeviceAddressFeatures = {};		//VK_KHR_buffer_device_address
			VkPhysicalDeviceTimelineSemaphoreFeatures			m_TimelineSemaphoreFeatures = {};		//VK_KHR_timeline_semaphore
			VkPhysicalDeviceTimelineSemaphoreProperties			m_TimelineSemaphoreProperties = {};		//VK_KHR_timeline_semaphore
			VkPhysicalDeviceSynchronization2Features			m_Synchronization2Features = {};		//VK_KHR_synchronization2
			VkPhysicalDeviceMeshShaderFeaturesEXT				m_DeviceMeshShaderFeatures = {};		//VK_EXT_mesh_shader
			VkPhysicalDeviceMeshShaderPropertiesEXT				m_DeviceMeshShaderProperties = {};		//VK_EXT_mesh_shader
			VkPhysicalDeviceDynamicRenderingFeatures			m_DynamicRenderingFeatures = {};		//VK_KHR_dynamic_rendering
			VkPhysicalDeviceMultiviewFeatures					m_MultivewFeatures = {};				//VK_KHR_multiview
			VkPhysicalDeviceMultiviewProperties					m_MultivewProperties = {};				//VK_KHR_multiview
			VkPhysicalDeviceShaderFloat16Int8Features			m_ShaderFloat16Int8Features = {};		//VK_KHR_shader_float16_int8
			VkPhysicalDevice16BitStorageFeatures				m_16BitStorageFeatures = {};			//VK_KHR_16bit_storage

			VkPhysicalDeviceVulkan11Features					m_Vulkan11Features = {};
			VkPhysicalDeviceVulkan11Properties					m_Vulkan11Properties = {};
			VkPhysicalDeviceVulkan12Features					m_Vulkan12Features = {};
			VkPhysicalDeviceVulkan12Properties					m_Vulkan12Properties = {};
			VkPhysicalDeviceVulkan13Features					m_Vulkan13Features = {};
			VkPhysicalDeviceVulkan13Properties					m_Vulkan13Properties = {};
			VkPhysicalDeviceVulkan14Features					m_Vulkan14Features = {};
			VkPhysicalDeviceVulkan14Properties					m_Vulkan14Properties = {};

			FeaturesAndProperties() = default;
			FeaturesAndProperties(const Device* device);
		};

		//Methods:
	public:
		Device(CreateInfo* pCreateInfo);
		~Device();

		void DeviceWaitIdle() override { vkDeviceWaitIdle(m_Device); };

	private:
		void AddExtensions();
		void SetResultInfo();
		void LoadDeviceExtensionPFNs();

		//Members
	public:
		//Device
		VkDevice m_Device;
		VkDeviceCreateInfo m_DeviceCI;
		std::vector<std::string> m_Layers;
		std::vector<std::string> m_Extensions;
		std::vector<const char*> m_ActiveLayers;
		std::vector<const char*> m_ActiveExtensions;

		//Queues
		std::vector<std::vector<VkQueue>> m_Queues;
		std::vector<VkDeviceQueueCreateInfo> m_DeviceQueueCIs;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<std::vector<float>> m_QueuePriorities;

		FeaturesAndProperties m_FeatureAndProperties;
	};
}
}