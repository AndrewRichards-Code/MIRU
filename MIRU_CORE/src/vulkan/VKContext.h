#pragma once
#if defined(MIRU_VULKAN)
#include "crossplatform/Context.h"

namespace miru
{
namespace vulkan
{
	class Context final : public crossplatform::Context
	{
		//enum/structs
	public:
		struct PhysicalDevices
		{
			std::vector<VkPhysicalDevice> m_PhysicalDevices;
			std::vector<VkPhysicalDeviceFeatures> m_PhysicalDeviceFeatures;
			std::vector<VkPhysicalDeviceProperties> m_PhysicalDeviceProperties;
			std::vector<VkPhysicalDeviceMemoryProperties> m_PhysicalDeviceMemoryProperties;

			#if defined(VK_VERSION_1_1)
			std::vector<VkPhysicalDeviceFeatures2> m_PhysicalDeviceFeatures2;
			std::vector<VkPhysicalDeviceProperties2> m_PhysicalDeviceProperties2;
			#endif
			#if defined(VK_VERSION_1_2)
			std::vector<VkPhysicalDeviceVulkan12Features> m_PhysicalDeviceVulkan12Features;
			#endif
			#if defined(VK_KHR_acceleration_structure)
			std::vector<VkPhysicalDeviceAccelerationStructureFeaturesKHR> m_PhysicalDeviceAccelerationStructureFeatures;
			std::vector<VkPhysicalDeviceAccelerationStructurePropertiesKHR> m_PhysicalDeviceAccelerationStructureProperties;
			#endif
			#if defined(VK_KHR_ray_tracing_pipeline)
			std::vector<VkPhysicalDeviceRayTracingPipelineFeaturesKHR> m_PhysicalDeviceRayTracingPipelineFeatures;
			std::vector<VkPhysicalDeviceRayTracingPipelinePropertiesKHR> m_PhysicalDeviceRayTracingPipelineProperties;
			#endif
			#if defined(VK_KHR_dynamic_rendering)
			std::vector<VkPhysicalDeviceDynamicRenderingFeaturesKHR> m_PhysicalDeviceDynamicRenderingFeaturesKHR;
			#endif

			PhysicalDevices(const VkInstance& instance, uint32_t apiVersion);

			PhysicalDevices() {};
			PhysicalDevices(const PhysicalDevices& physicalDevice) 
			{
				m_PhysicalDevices = physicalDevice.m_PhysicalDevices;
				m_PhysicalDeviceFeatures = physicalDevice.m_PhysicalDeviceFeatures;
				m_PhysicalDeviceProperties = physicalDevice.m_PhysicalDeviceProperties;
				m_PhysicalDeviceMemoryProperties = physicalDevice.m_PhysicalDeviceMemoryProperties;

				#if defined(VK_VERSION_1_1)
				m_PhysicalDeviceFeatures2 = physicalDevice.m_PhysicalDeviceFeatures2;
				m_PhysicalDeviceProperties2 = physicalDevice.m_PhysicalDeviceProperties2;
				#endif
				#if defined(VK_VERSION_1_2)
				m_PhysicalDeviceVulkan12Features = physicalDevice.m_PhysicalDeviceVulkan12Features;
				#endif
				#if defined(VK_KHR_ray_tracing_pipeline)
				m_PhysicalDeviceAccelerationStructureFeatures = physicalDevice.m_PhysicalDeviceAccelerationStructureFeatures;
				m_PhysicalDeviceAccelerationStructureProperties = physicalDevice.m_PhysicalDeviceAccelerationStructureProperties;
				#endif
				#if defined(VK_KHR_ray_tracing_pipeline)
				m_PhysicalDeviceRayTracingPipelineFeatures = physicalDevice.m_PhysicalDeviceRayTracingPipelineFeatures;
				m_PhysicalDeviceRayTracingPipelineProperties = physicalDevice.m_PhysicalDeviceRayTracingPipelineProperties;
				#endif
			};
		};

		//Methods
	public:
		Context(Context::CreateInfo* pCreateInfo);
		~Context();

		void* GetDevice() override { return &m_Device; }
		void DeviceWaitIdle() override { vkDeviceWaitIdle(m_Device); };

		bool IsActive(std::vector<const char*> list, const char* name);

		//Members
	public:
		//Instance
		VkInstance m_Instance;
		VkApplicationInfo m_AI;
		VkInstanceCreateInfo m_InstanceCI;
		std::vector<const char*> m_ActiveInstanceLayers;
		std::vector<const char*> m_ActiveInstanceExtensions;

		//Device
		VkDevice m_Device;
		VkDeviceCreateInfo m_DeviceCI;
		std::vector<const char*> m_ActiveDeviceLayers;
		std::vector<const char*> m_ActiveDeviceExtensions;
		PhysicalDevices m_PhysicalDevices;

		//Queues
		std::vector<std::vector<VkQueue>> m_Queues;
		std::vector<VkDeviceQueueCreateInfo> m_DeviceQueueCIs;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<std::vector<float>> m_QueuePriorities;
	};
}
}
#endif