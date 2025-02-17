#pragma once

#include "miru_core_common.h"
#include "PhysicalDevice.h"

namespace miru
{
namespace base
{
	class MIRU_API Instance
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			std::string		applicationName;
			bool			debugValidationLayers;
			void*			pNext;
		};
		struct ResultInfo
		{
			uint32_t		apiVersionMajor;
			uint32_t		apiVersionMinor;
			uint32_t		apiVersionPatch;
		};
		enum class CreateInfoExtensionStructureTypes : uint32_t
		{
			UNKNOWN,
			OPENXR_D3D12_DATA,
			OPENXR_VULKAN_DATA,
		};

		//Methods
	public:
		static InstanceRef Create(CreateInfo* pCreateInfo);
		virtual ~Instance() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		const ResultInfo& GetResultInfo() { return m_RI; }

		virtual PhysicalDeviceRefs GetPhysicalDevices() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
		ResultInfo m_RI = {};
	};
}
}