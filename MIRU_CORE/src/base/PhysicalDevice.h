#pragma once

#include "miru_core_common.h"

namespace miru
{
namespace base
{
	class MIRU_API PhysicalDevice
	{
		//enums/structs
	public:
		typedef void* NativeHandle;
		struct CreateInfo
		{
			InstanceRef		instance;
			NativeHandle	nativeHandle;
		};

		//Methods
	public:
		static PhysicalDeviceRef Create(CreateInfo* pCreateInfo);
		virtual ~PhysicalDevice() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		InstanceRef GetInstance() { return m_CI.instance; }
		const bool& OpenXRValid() { return m_OpenXRValid; }

		//Members
	public:
		CreateInfo m_CI;
		bool m_OpenXRValid = false;
	};

	typedef std::vector<PhysicalDeviceRef> PhysicalDeviceRefs;
}
}