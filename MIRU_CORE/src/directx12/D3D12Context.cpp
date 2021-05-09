#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Context.h"
#include "D3D12Sync.h"

using namespace miru;
using namespace d3d12;

HMODULE Context::s_HModeuleDXIL;
std::filesystem::path Context::s_DXILFullpath;
uint32_t Context::s_RefCount = 0;

Context::Context(Context::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	//Setup Debug
#if defined(_DEBUG)
	MIRU_ASSERT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug)), "ERROR: D3D12: Failed to get DebugInterface.");
	m_Debug->EnableDebugLayer();
	#if !defined(MIRU_WIN64_UWP)
	reinterpret_cast<ID3D12Debug1*>(m_Debug)->SetEnableGPUBasedValidation(true);
	#endif
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
	if (!s_HModeuleDXIL)
	{
		s_DXILFullpath = std::string(PROJECT_DIR) + "redist/dxc/lib/x64/dxil.dll";
		s_HModeuleDXIL = arc::DynamicLibrary::Load(s_DXILFullpath.generic_string());
		if (!s_HModeuleDXIL)
		{
			std::string error_str = "WARN: D3D12: Unable to load '" + s_DXILFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
	s_RefCount++;

	//Check provide feature level
	uint32_t featureLevel = D3D_FEATURE_LEVEL_11_0;
	if (m_CI.api_version_major == 12)
		featureLevel += 0x1000;
	if (m_CI.api_version_minor == 1)
		featureLevel += 0x0100;

	D3D_FEATURE_LEVEL featureLevels[4] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	HRESULT res = D3D12CreateDevice(m_PhysicalDevices.m_Adapters[0], (D3D_FEATURE_LEVEL)featureLevel, __uuidof(ID3D12Device), nullptr);
	if (res != S_OK)
	{
		for (size_t i = 0; i < 4; i++)
		{
			featureLevel = featureLevels[i];
			res = D3D12CreateDevice(m_PhysicalDevices.m_Adapters[0], (D3D_FEATURE_LEVEL)featureLevel, __uuidof(ID3D12Device), nullptr);
			if (res == S_FALSE)
				break;
			else
				continue;
		}
	}

	//Create Device
	MIRU_ASSERT(D3D12CreateDevice(m_PhysicalDevices.m_Adapters[0], (D3D_FEATURE_LEVEL)featureLevel, IID_PPV_ARGS(&m_Device)), "ERROR: D3D12: Failed to create Device."); //We only use the first PhysicalDevice
	D3D12SetName(m_Device, m_CI.deviceDebugName);

	//Create Info Queue
	m_Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue));
	if (m_InfoQueue)
	{
		m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

		D3D12_MESSAGE_ID filteredMessageIDs[2] = { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE };
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.pIDList = filteredMessageIDs;
		filter.DenyList.NumIDs = _countof(filteredMessageIDs);
		m_InfoQueue->AddStorageFilterEntries(&filter);
	}

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
		D3D12SetName(m_Queues[i], m_CI.deviceDebugName + ": Queue - " + typeStr);
	}
}

Context::~Context()
{
	MIRU_CPU_PROFILE_FUNCTION();

	s_RefCount--;
	if (!s_RefCount)
	{
		if (!FreeLibrary(s_HModeuleDXIL))
		{
			std::string error_str = "WARN: D3D12: Unable to free'" + s_DXILFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
	ID3D12DebugDevice* debugDevice;
	m_Device->QueryInterface(&debugDevice);

	for (auto& queue : m_Queues)
		MIRU_D3D12_SAFE_RELEASE(queue);

	MIRU_D3D12_SAFE_RELEASE(m_Device);

	for (auto& adapter : m_PhysicalDevices.m_Adapters)
		MIRU_D3D12_SAFE_RELEASE(adapter);

	MIRU_D3D12_SAFE_RELEASE(m_Factory);

	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
	debugDevice->Release();
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
		fence->Wait();
		
		fence->~Fence();
		fence = nullptr;
	}
}
#endif