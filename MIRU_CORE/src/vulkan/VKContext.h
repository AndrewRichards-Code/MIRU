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
			struct PhysicalDeviceInfo
			{
				VkPhysicalDevice m_PhysicalDevice;
				VkPhysicalDeviceFeatures m_Features;
				VkPhysicalDeviceProperties m_Properties;
				VkPhysicalDeviceMemoryProperties m_MemoryProperties;

				#if defined(VK_KHR_get_physical_device_properties2)
				VkPhysicalDeviceFeatures2 m_Features2;
				VkPhysicalDeviceProperties2 m_Properties2;
				#endif
				#if defined(VK_KHR_buffer_device_address)
				VkPhysicalDeviceBufferDeviceAddressFeatures m_BufferDeviceAddressFeatures;
				#endif
				#if defined(VK_KHR_ray_tracing_pipeline)
				VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_RayTracingPipelineFeatures;
				VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingPipelineProperties;
				#endif
				#if defined(VK_KHR_acceleration_structure)
				VkPhysicalDeviceAccelerationStructureFeaturesKHR m_AccelerationStructureFeatures;
				VkPhysicalDeviceAccelerationStructurePropertiesKHR m_AccelerationStructureProperties;
				#endif
				#if defined(VK_KHR_dynamic_rendering)
				VkPhysicalDeviceDynamicRenderingFeaturesKHR m_DynamicRenderingFeatures;
				#endif
			};
			std::vector<PhysicalDeviceInfo> m_PDIs;

			PhysicalDevices(const VkInstance& instance, uint32_t apiVersion);
			PhysicalDevices() {};
			PhysicalDevices(const PhysicalDevices& physicalDevice) 
			{
				for (const PhysicalDeviceInfo& physicalDeviceInfo : physicalDevice.m_PDIs)
				{
					m_PDIs.push_back(physicalDeviceInfo);
				}
			};

			void FillOutFeaturesAndProperties(Context* pContext);
		};

		//Methods
	public:
		Context(Context::CreateInfo* pCreateInfo);
		~Context();

		void* GetDevice() override { return &m_Device; }
		void DeviceWaitIdle() override { vkDeviceWaitIdle(m_Device); };

		static bool IsActive(std::vector<const char*> list, const char* name);

		//Members
	public:
		//Instance
		VkInstance m_Instance;
		VkApplicationInfo m_AI;
		VkInstanceCreateInfo m_InstanceCI;
		std::vector<std::string> m_InstanceLayers;
		std::vector<std::string> m_InstanceExtensions;
		std::vector<const char*> m_ActiveInstanceLayers;
		std::vector<const char*> m_ActiveInstanceExtensions;

		//Device
		VkDevice m_Device;
		VkDeviceCreateInfo m_DeviceCI;
		std::vector<std::string> m_DeviceLayers;
		std::vector<std::string> m_DeviceExtensions;
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