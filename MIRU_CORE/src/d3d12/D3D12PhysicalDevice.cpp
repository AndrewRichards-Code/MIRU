#include "d3d12/D3D12PhysicalDevice.h"
#include "d3d12/D3D12Instance.h"

using namespace miru;
using namespace d3d12;

PhysicalDevice::PhysicalDevice(CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	InstanceRef instance = ref_cast<Instance>(GetInstance());

	reinterpret_cast<IDXGIAdapter1*>(m_CI.nativeHandle)->QueryInterface(IID_PPV_ARGS(&m_Adapter));
	m_Adapter->GetDesc3(&m_AdapterDesc);

	//OpenXR Data
	Instance::OpenXRD3D12Data* openXRD3D12Data = reinterpret_cast<Instance::OpenXRD3D12Data*>(instance->GetCreateInfo().pNext);
	if (!(openXRD3D12Data && openXRD3D12Data->type == Instance::CreateInfoExtensionStructureTypes::OPENXR_D3D12_DATA))
		openXRD3D12Data = nullptr;
	
	if (openXRD3D12Data)
	{
		const LUID& openXRAdapterLuid = openXRD3D12Data->adapterLuid;
		const LUID& adapterLuid = m_AdapterDesc.AdapterLuid;
		m_OpenXRValid = memcmp(&openXRAdapterLuid, &adapterLuid, sizeof(LUID)) == 0;
	}

	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (m_Adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		if (output)
		{
			m_Outputs.push_back({});
			std::pair<IDXGIOutput6*, DXGI_OUTPUT_DESC1>& _output = m_Outputs.back();

			IDXGIOutput6*& output6 = _output.first;
			output->QueryInterface(IID_PPV_ARGS(&output6));
			DXGI_OUTPUT_DESC1& outputDesc = _output.second;
			output6->GetDesc1(&outputDesc);
		}
		i++;
	}
}

PhysicalDevice::~PhysicalDevice()
{
	for (auto& output : m_Outputs)
		MIRU_D3D12_SAFE_RELEASE(output.first);

	MIRU_D3D12_SAFE_RELEASE(m_Adapter);

	MIRU_CPU_PROFILE_FUNCTION();
}