#pragma once

#include "base/PhysicalDevice.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class PhysicalDevice final : public base::PhysicalDevice
	{
		//Method
	public:
		PhysicalDevice(CreateInfo* pCreateInfo);
		~PhysicalDevice();

		//Member
	public: 
		VkPhysicalDevice m_PhysicalDevice;
		VkPhysicalDeviceFeatures m_Features = {};
		VkPhysicalDeviceProperties m_Properties = {};
		VkPhysicalDeviceMemoryProperties m_MemoryProperties = {};

	};
}
}