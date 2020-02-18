#pragma once
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

			PhysicalDevices(const VkInstance& instance);

			PhysicalDevices() {};
			PhysicalDevices(const PhysicalDevices& physicalDevice) 
			{
				m_PhysicalDevices = physicalDevice.m_PhysicalDevices;
				m_PhysicalDeviceFeatures = physicalDevice.m_PhysicalDeviceFeatures;
				m_PhysicalDeviceProperties = physicalDevice.m_PhysicalDeviceProperties;
				m_PhysicalDeviceMemoryProperties = physicalDevice.m_PhysicalDeviceMemoryProperties;
			};
		};

		//Methods
	public:
		Context(Context::CreateInfo* pCreateInfo);
		~Context();

		void* GetDevice() override { return &m_Device; }
		void DeviceWaitIdle() override { vkDeviceWaitIdle(m_Device); };


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