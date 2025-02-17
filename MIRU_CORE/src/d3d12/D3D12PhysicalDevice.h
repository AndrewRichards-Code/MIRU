#pragma once

#include "base/PhysicalDevice.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class PhysicalDevice final : public base::PhysicalDevice
	{
		//Methods
	public:
		PhysicalDevice(PhysicalDevice::CreateInfo* pCreateInfo);
		~PhysicalDevice();

		//Members
	public:
		IDXGIAdapter4* m_Adapter;
		DXGI_ADAPTER_DESC3 m_AdapterDesc;
		std::vector<std::pair<IDXGIOutput6*, DXGI_OUTPUT_DESC1>> m_Outputs;
	};
}
}