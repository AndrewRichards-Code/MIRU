#pragma once

#include "base/Instance.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class Instance final : public base::Instance
	{
		//enum/structs
	public:
		struct OpenXRD3D12Data
		{
			CreateInfoExtensionStructureTypes	type;
			void*								pNext;
			LUID								adapterLuid;
			D3D_FEATURE_LEVEL					minFeatureLevel;
		};

		//Methods
	public:
		Instance(Instance::CreateInfo* pCreateInfo);
		~Instance();

		base::PhysicalDeviceRefs GetPhysicalDevicesInternal(base::InstanceRef instance) override;

		//Member
	public:
		//Debug
		ID3D12Debug* m_Debug = nullptr;
		ID3D12InfoQueue* m_InfoQueue = nullptr;
		DWORD m_CallbackCookie = 0;

		//Factory
		IDXGIFactory7* m_Factory = nullptr;
	};
}
}