#include "D3D12Instance.h"
#include "D3D12PhysicalDevice.h"

using namespace miru;
using namespace d3d12;

Instance::Instance(Instance::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//Setup Debug
	if (m_CI.debugValidationLayers)
	{
		MIRU_FATAL(D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug)), "ERROR: D3D12: Failed to get DebugInterface.");
		m_Debug->EnableDebugLayer();
		#if !defined(MIRU_WIN64_UWP)
		reinterpret_cast<ID3D12Debug1*>(m_Debug)->SetEnableGPUBasedValidation(true);
		#endif
	}

	//Create Factory
	UINT createFactoryFlags = 0;
	if (m_CI.debugValidationLayers)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	MIRU_FATAL(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_Factory)), "ERROR: D3D12: Failed to create IDXGIFactory7.");
}

Instance::~Instance()
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_D3D12_SAFE_RELEASE(m_Factory);
}

base::PhysicalDeviceRefs Instance::GetPhysicalDevicesInternal(base::InstanceRef instance)
{
	MIRU_CPU_PROFILE_FUNCTION();

	base::PhysicalDeviceRefs physicalDevices;

	UINT i = 0;
	IDXGIAdapter1* adapter = nullptr;
	while (m_Factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		if (adapter)
		{
			base::PhysicalDevice::CreateInfo physicalDeviceCI;
			physicalDeviceCI.instance = instance;
			physicalDeviceCI.nativeHandle = adapter;
			physicalDevices.push_back(base::PhysicalDevice::Create(&physicalDeviceCI));
		}
		
		MIRU_D3D12_SAFE_RELEASE(adapter);
		i++;
	}

	return physicalDevices;
}