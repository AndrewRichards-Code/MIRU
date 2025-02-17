#pragma once

#include "base/Instance.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class Instance final : public base::Instance
	{
		//enum/structs
	public:
		struct OpenXRVulkanData
		{
			CreateInfoExtensionStructureTypes			type;
			void*										pNext;
			uint16_t									minApiVersionMajorSupported;
			uint16_t									minApiVersionMinorSupported;
			uint16_t									maxApiVersionMajorSupported;
			uint16_t									maxApiVersionMinorSupported;
			std::vector<std::string>					instanceExtensions;
			std::vector<std::string>					deviceExtensions;
			std::function<VkPhysicalDevice(VkInstance)>	getPhysicalDeviceVulkan;
		};

		//Methods
	public:
		Instance(Instance::CreateInfo* pCreateInfo);
		~Instance();

		base::PhysicalDeviceRefs GetPhysicalDevices() override;

		static bool IsActive(std::vector<const char*> list, const char* name);

		static VkBool32 MessageCallbackFunction(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	private:
		void AddExtensions();
		void SetResultInfo();
		void LoadInstanceExtensionPFNs();

		//Members
	public:
		//Instance
		VkInstance m_Instance;
		VkApplicationInfo m_AI;
		VkInstanceCreateInfo m_InstanceCI;
		std::vector<std::string> m_Layers;
		std::vector<std::string> m_Extensions;
		std::vector<const char*> m_ActiveLayers;
		std::vector<const char*> m_ActiveExtensions;

		//DebugMessenger
		VkDebugUtilsMessengerCreateInfoEXT m_DebugUtilsMessengerCI;
		VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
	};
}
}