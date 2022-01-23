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
		enum class ExtensionsBit : uint32_t
		{
			NONE				= 0x00000000,
			RAY_TRACING			= 0x00000001,
			DYNAMIC_RENDERING	= 0x00000002
		};
		struct CreateInfo
		{
			std::string		applicationName;
			bool			debugValidationLayers;
			ExtensionsBit	extensions;
			std::string		deviceDebugName;
		};
		struct ResultInfo
		{
			uint32_t		apiVersionMajor;
			uint32_t		apiVersionMinor;
			uint32_t		apiVersionPatch;
			ExtensionsBit	activeExtensions;
		};

		//Methods
	public:
		static Ref<Context> Create(CreateInfo* pCreateInfo);
		virtual ~Context() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		const ResultInfo& GetResultInfo() { return m_RI; }

		virtual void* GetDevice() = 0;
		virtual void DeviceWaitIdle() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
		ResultInfo m_RI = {};
	};
}
}