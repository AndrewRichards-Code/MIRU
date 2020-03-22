#include "common.h"
#include "D3D12Context.h"
#include "D3D12Sync.h"

using namespace miru;
using namespace d3d12;

Context::Context(Context::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//Setup Debug
	#if defined(_DEBUG)
	MIRU_ASSERT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug)), "ERROR: D3D12: Failed to get DebugInterface.");
	m_Debug->EnableDebugLayer();
	reinterpret_cast<ID3D12Debug1*>(m_Debug)->SetEnableGPUBasedValidation(true);
	#endif

	//Create Factory
	UINT createFactoryFlags = 0;
	#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	#endif
	MIRU_ASSERT(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_Factory)), "ERROR: D3D12: Failed to create IDXGIFactory4.");
	
	//Create PhysicalDevices
	m_PhysicalDevices = PhysicalDevices(m_Factory);

	//Load dxil.dll
	HMODULE dxil = LoadLibrary("C:/Program Files (x86)/Windows Kits/10/Redist/D3D/x64/dxil.dll");
	if (!dxil)
		MIRU_WARN(GetLastError(), "WARN: D3D12: Unable to load 'C:/Program Files (x86)/Windows Kits/10/Redist/D3D/x64/dxil.dll'.");
	
	//Create Device
	uint32_t featureLevel = D3D_FEATURE_LEVEL_11_0;
	if (m_CI.api_version_major == 12)
		featureLevel += 0x1000;
	if (m_CI.api_version_minor == 1)
		featureLevel += 0x0100;

	MIRU_ASSERT(D3D12CreateDevice(m_PhysicalDevices.m_Adapters[0], (D3D_FEATURE_LEVEL)featureLevel, IID_PPV_ARGS(&m_Device)), "ERROR: D3D12: Failed to create Device."); //We only use the first PhysicalDevice
	D3D12SetName(m_Device, m_CI.deviceDebugName);

	//Create Queues
	m_QueueDescs.resize(3);
	m_Queues.resize(3);

	for (size_t i = 0; i < m_Queues.size(); i++)
	{
		m_QueueDescs[i].Type = i == 0 ? D3D12_COMMAND_LIST_TYPE_DIRECT : i == 1 ? D3D12_COMMAND_LIST_TYPE_COMPUTE : i == 2 ? D3D12_COMMAND_LIST_TYPE_COPY : D3D12_COMMAND_LIST_TYPE_DIRECT;
		m_QueueDescs[i].Priority = 0;
		m_QueueDescs[i].Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		m_QueueDescs[i].NodeMask = 0;
		MIRU_ASSERT(m_Device->CreateCommandQueue(&m_QueueDescs[i], IID_PPV_ARGS(&m_Queues[i])), "ERROR: D3D12: Failed to create CommandQueue.");

		std::string typeStr = i == 0 ? "Direct" : i == 1 ? "Compute" : i == 2 ? "Copy" : "";
		D3D12SetName(m_Queues[i], (std::string(m_CI.deviceDebugName) + ": Queue - " + typeStr).c_str());
	}
}

Context::~Context()
{
	MIRU_CPU_PROFILE_FUNCTION();

	for (auto& queue : m_Queues)
		SAFE_RELEASE(queue);

	SAFE_RELEASE(m_Device);

	for (auto& adapter : m_PhysicalDevices.m_Adapters)
		SAFE_RELEASE(adapter);

	SAFE_RELEASE(m_Factory);
}

Context::PhysicalDevices::PhysicalDevices(IDXGIFactory4* factory)
{
	MIRU_CPU_PROFILE_FUNCTION();

	UINT i = 0;
	IDXGIAdapter1* adapter;
	DXGI_ADAPTER_DESC adapterDesc = {};
	while (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		m_Adapters.push_back(reinterpret_cast<IDXGIAdapter4*>(adapter));
		adapter->GetDesc(&adapterDesc);
		m_AdapterDescs.push_back(adapterDesc);
		i++;
	}
}

void Context::DeviceWaitIdle()
{
	MIRU_CPU_PROFILE_FUNCTION();

	Ref<crossplatform::Fence> fence;
	Fence::CreateInfo ci;
	ci.debugName = "";
	ci.device = m_Device;
	ci.signaled = false;
	ci.timeout = UINT64_MAX; //In nanoseconds
	
	for (auto& queue : m_Queues)
	{
		fence = Fence::Create(&ci);
		ref_cast<Fence>(fence)->GetValue()++;

		queue->Signal(ref_cast<Fence>(fence)->m_Fence, ref_cast<Fence>(fence)->GetValue());
		while (!fence->Wait()) {}
		fence->~Fence();
		fence = nullptr;
	}

}
