#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace crossplatform
{
	class Context
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			const char*					applicationName = nullptr;
			uint32_t					api_version_major = 0;
			uint32_t					api_version_minor = 0;
			std::vector<std::string>	instanceLayers;
			std::vector<std::string>	instanceExtensions;
			std::vector<std::string>	deviceLayers;
			std::vector<std::string>	deviceExtensions;
			const char*					deviceDebugName = nullptr;
		};

		//Methods
	public:
		static Ref<Context> Create(CreateInfo* pCreateInfo);
		virtual ~Context() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void* GetDevice() = 0;
		virtual void DeviceWaitIdle() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}