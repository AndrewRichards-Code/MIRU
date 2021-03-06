#pragma once
#if defined(MIRU_D3D12)
#include "crossplatform/Context.h"

namespace miru
{
namespace d3d12
{
	class Context final : public crossplatform::Context
	{
		//enum/structs
	public:
		struct PhysicalDevices
		{
			std::vector<IDXGIAdapter4*> m_Adapters;
			std::vector<DXGI_ADAPTER_DESC> m_AdapterDescs;

			PhysicalDevices(IDXGIFactory4* factory);

			PhysicalDevices() {};
			PhysicalDevices(const PhysicalDevices& physicalDevice)
			{
				m_Adapters = physicalDevice.m_Adapters;
				m_AdapterDescs = physicalDevice.m_AdapterDescs;
			};
		};

		//Methods
	public:
		Context(Context::CreateInfo* pCreateInfo);
		~Context();

		void* GetDevice() override { return m_Device; }
		void DeviceWaitIdle() override;

		//Member
	public:
		//Debug
		ID3D12Debug* m_Debug;
		ID3D12InfoQueue* m_InfoQueue;

		//Factory
		IDXGIFactory4* m_Factory;

		//Device
		ID3D12Device* m_Device;
		PhysicalDevices m_PhysicalDevices;

		//Queue
		std::vector<ID3D12CommandQueue*> m_Queues;
		std::vector<D3D12_COMMAND_QUEUE_DESC> m_QueueDescs;

	private:
		//DXIL Library
		static HMODULE s_HModeuleDXIL;
		static std::filesystem::path s_DXILFullpath;
		static uint32_t s_RefCount;
	};
}
}
#endif