#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Context.h"
#include "D3D12Sync.h"
#include "D3D12Shader.h"

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
	if (m_CI.debugValidationLayers)
	{
		MIRU_ASSERT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug)), "ERROR: D3D12: Failed to get DebugInterface.");
		m_Debug->EnableDebugLayer();
		#if !defined(MIRU_WIN64_UWP)
		reinterpret_cast<ID3D12Debug1*>(m_Debug)->SetEnableGPUBasedValidation(true);
		#endif
	}

	//Create Factory
	UINT createFactoryFlags = 0;
	if (m_CI.debugValidationLayers)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	MIRU_ASSERT(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_Factory)), "ERROR: D3D12: Failed to create IDXGIFactory4.");

	//Create PhysicalDevices
	m_PhysicalDevices = PhysicalDevices(m_Factory);

	//Load dxil.dll
	if (!s_HModeuleDXIL)
	{
		s_DXILFullpath = base::Shader::GetLibraryFullpath_dxil();
		s_HModeuleDXIL = base::Shader::LoadLibrary_dxil();
		if (!s_HModeuleDXIL)
		{
			std::string error_str = "WARN: D3D12: Unable to load '" + s_DXILFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
	s_RefCount++;

	//Check provide feature level
	D3D_FEATURE_LEVEL featureLevel;
	for (size_t i = 0; i < _countof(m_Features.featureLevelsList); i++)
	{
		featureLevel = m_Features.featureLevelsList[i];
		HRESULT res = D3D12CreateDevice(m_PhysicalDevices.m_PDIs[0].m_Adapter, featureLevel, __uuidof(ID3D12Device), nullptr);
		if (res == S_FALSE)
			break;
		else
			continue;
	}
	m_RI.apiVersionMajor = (((uint32_t)(featureLevel) >> 12) & 0xFU);
	m_RI.apiVersionMinor = (((uint32_t)(featureLevel) >> 8) & 0xFU);
	m_RI.apiVersionPatch = 0;
	
	//Create Device
	MIRU_ASSERT(D3D12CreateDevice(m_PhysicalDevices.m_PDIs[0].m_Adapter, featureLevel, IID_PPV_ARGS(&m_Device)), "ERROR: D3D12: Failed to create Device."); //We only use the first PhysicalDevice
	D3D12SetName(m_Device, m_CI.deviceDebugName);

	//Enumerate D3D12 Device Features
	m_Features = Features(m_Device);

	m_RI.activeExtensions = ExtensionsBit::DYNAMIC_RENDERING;
	if (m_Features.d3d12Options5.RaytracingTier > D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		m_RI.activeExtensions |= ExtensionsBit::RAY_TRACING;
	if (m_Features.d3d12Options12.EnhancedBarriersSupported)
		m_RI.activeExtensions |= ExtensionsBit::SYNCHRONISATION_2;
	if (m_Features.d3d12Options7.MeshShaderTier > D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
		m_RI.activeExtensions |= ExtensionsBit::MESH_SHADER;
	if (m_Features.d3d12Options3.ViewInstancingTier > D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED)
		m_RI.activeExtensions |= ExtensionsBit::MULTIVIEW;
	if (m_Features.d3d12Options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation)
		m_RI.activeExtensions |= ExtensionsBit::SHADER_VIEWPORT_INDEX_LAYER;
	

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

		reinterpret_cast<ID3D12InfoQueue1*>(m_InfoQueue)->RegisterMessageCallback(MessageCallbackFunction, D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &m_CallbackCookie);
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

	if (m_InfoQueue)
		reinterpret_cast<ID3D12InfoQueue1*>(m_InfoQueue)->UnregisterMessageCallback(m_CallbackCookie);

	ID3D12DebugDevice* debugDevice = nullptr;
	m_Device->QueryInterface(&debugDevice);

	for (auto& queue : m_Queues)
		MIRU_D3D12_SAFE_RELEASE(queue);

	MIRU_D3D12_SAFE_RELEASE(m_Device);

	for (auto& physicalDeviceInfo : m_PhysicalDevices.m_PDIs)
		MIRU_D3D12_SAFE_RELEASE(physicalDeviceInfo.m_Adapter);

	MIRU_D3D12_SAFE_RELEASE(m_Factory);

	if (debugDevice)
	{
		debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
		debugDevice->Release();
	}
}

Context::PhysicalDevices::PhysicalDevices(IDXGIFactory4* factory)
{
	MIRU_CPU_PROFILE_FUNCTION();

	UINT i = 0;
	IDXGIAdapter1* adapter;
	DXGI_ADAPTER_DESC adapterDesc = {};
	while (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapter->GetDesc(&adapterDesc);
		m_PDIs.push_back({ reinterpret_cast<IDXGIAdapter4*>(adapter), adapterDesc });
		i++;
	}
}

void Context::DeviceWaitIdle()
{
	MIRU_CPU_PROFILE_FUNCTION();

	base::FenceRef fence;
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

template <>
struct magic_enum::customize::enum_range<D3D12_MESSAGE_ID> {
	static constexpr int min = static_cast<int>(D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_UNKNOWN);
	static constexpr int max = static_cast<int>(D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_D3D12_MESSAGES_END);
};

void Context::MessageCallbackFunction(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext)
{
	std::string category = std::string(magic_enum::enum_name<D3D12_MESSAGE_CATEGORY>(Category));
	std::string severity = std::string(magic_enum::enum_name<D3D12_MESSAGE_SEVERITY>(Severity));
	//std::string id = std::string(magic_enum::enum_name<D3D12_MESSAGE_ID>(ID));
	std::string description = pDescription;

	category = category.substr(category.find_last_of('_') + 1);
	severity = severity.substr(severity.find_last_of('_') + 1);

	std::stringstream errorMessage;
	errorMessage << "D3D12 " << severity << ": " << description << " [ " << severity << " " << category << " #" << uint32_t(ID) << ": " /*<< id*/ << " ]";
	std::string errorMessageStr = errorMessage.str();

	switch (Severity)
	{
		case D3D12_MESSAGE_SEVERITY_CORRUPTION:
		{
			MIRU_FATAL(uint32_t(ID), errorMessageStr.c_str());
			break;
		}
		case D3D12_MESSAGE_SEVERITY_ERROR:
		{
			MIRU_ERROR(uint32_t(ID), errorMessageStr.c_str());
			break;
		}
		case D3D12_MESSAGE_SEVERITY_WARNING:
		{
			MIRU_WARN(uint32_t(ID), errorMessageStr.c_str());
			break;
		}
		case D3D12_MESSAGE_SEVERITY_INFO:
		case D3D12_MESSAGE_SEVERITY_MESSAGE:
		{
			MIRU_INFO(uint32_t(ID), errorMessageStr.c_str());
			break;
		}
	}
}

Context::Features::Features(ID3D12Device* device)
{
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12Options, sizeof(d3d12Options)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS.");

	featureLevels.NumFeatureLevels = _countof(featureLevelsList);
	featureLevels.pFeatureLevelsRequested = featureLevelsList;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_FEATURE_LEVELS.");

	for (uint32_t i = 0; i <= uint32_t(DXGI_FORMAT_B4G4R4A4_UNORM); i++)
	{
		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { DXGI_FORMAT(i), D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
		HRESULT res = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));
		if (res != S_OK && res != E_FAIL && res != E_INVALIDARG)
		{
			MIRU_WARN(res, "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_FORMAT_SUPPORT.");
		}
		formatSupports.push_back(formatSupport);

		std::vector<D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS> _multisampleQualityLevels;
		for (base::Image::SampleCountBit j = base::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
			j <= base::Image::SampleCountBit::SAMPLE_COUNT_8_BIT; j = base::Image::SampleCountBit(uint32_t(j) * 2))
		{
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleQualityLevel = { DXGI_FORMAT(i), UINT(j) };
			res = device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multisampleQualityLevel, sizeof(multisampleQualityLevel));
			if (res != S_OK && res != E_FAIL && res != E_INVALIDARG)
			{
				MIRU_WARN(res, "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS.");
			}
			_multisampleQualityLevels.push_back(multisampleQualityLevel);
		}
		multisampleQualityLevels.push_back(_multisampleQualityLevels);

		D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = { DXGI_FORMAT(i), 0 };
		res = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo));
		if (res != S_OK && res != E_FAIL && res != E_INVALIDARG)
		{
			MIRU_WARN(res, "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_FORMAT_INFO.");
		}
		formatInfos.push_back(formatInfo);
	}

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &gpuVirtualAddressSupport, sizeof(gpuVirtualAddressSupport)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT.");

	shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_7;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_MODEL.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &d3d12Options1, sizeof(d3d12Options1)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS1.");

	protectedResourceSessionSupport.NodeIndex = 0;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, &protectedResourceSessionSupport, sizeof(protectedResourceSessionSupport)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT.");

	rootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignature, sizeof(rootSignature)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_ROOT_SIGNATURE.");

	architecture1.NodeIndex = 0;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &architecture1, sizeof(architecture1)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_ARCHITECTURE1.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &d3d12Options2, sizeof(d3d12Options2)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS2.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &shaderCache, sizeof(shaderCache)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_CACHE.");

	for (uint32_t i = 0; i <= D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE; i++)
	{
		std::vector<D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY> _commandQueuePriorities;
		for (uint32_t j = 0; j < 1000000; j *= 100)
		{
			D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY commandQueuePriority = { D3D12_COMMAND_LIST_TYPE(i), UINT(D3D12_COMMAND_QUEUE_PRIORITY(j)), false };
			MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_COMMAND_QUEUE_PRIORITY, &commandQueuePriority, sizeof(commandQueuePriority)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_COMMAND_QUEUE_PRIORITY.");
			_commandQueuePriorities.push_back(commandQueuePriority);
			j = (j == 0 ? 1 : j);
		}
		commandQueuePriorities.push_back(_commandQueuePriorities);
	}

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &d3d12Options3, sizeof(d3d12Options3)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS3.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &existingHeaps, sizeof(existingHeaps)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_EXISTING_HEAPS.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &d3d12Options4, sizeof(d3d12Options4)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS4.");

	serialisation.NodeIndex = 0;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_SERIALIZATION, &serialisation, sizeof(serialisation)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SERIALIZATION.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_CROSS_NODE, &crossNode, sizeof(crossNode)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_CROSS_NODE.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12Options5, sizeof(d3d12Options5)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS5.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_DISPLAYABLE, &displayable, sizeof(displayable)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_DISPLAYABLE.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &d3d12Options6, sizeof(d3d12Options6)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS6.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &d3d12Options7, sizeof(d3d12Options7)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS7.");

	protectedResourceSessionTypeCount.NodeIndex = 0;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT, &protectedResourceSessionTypeCount, sizeof(protectedResourceSessionTypeCount)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT.");

	protectedResourceSessionTypes.NodeIndex = protectedResourceSessionTypeCount.NodeIndex;
	protectedResourceSessionTypes.Count = protectedResourceSessionTypeCount.Count;
	protectedResourceSessionTypesGuids.resize(protectedResourceSessionTypes.Count);
	protectedResourceSessionTypes.pTypes = protectedResourceSessionTypesGuids.data();
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPES, &protectedResourceSessionTypes, sizeof(protectedResourceSessionTypes)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPES.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS8, &d3d12Options8, sizeof(d3d12Options8)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS8.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &d3d12Options9, sizeof(d3d12Options9)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS9.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS10, &d3d12Options10, sizeof(d3d12Options10)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS10.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS11, &d3d12Options11, sizeof(d3d12Options11)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS11.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &d3d12Options12, sizeof(d3d12Options12)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS12.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS13, &d3d12Options13, sizeof(d3d12Options13)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS13.");
	
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS14, &d3d12Options14, sizeof(d3d12Options14)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS14.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS15, &d3d12Options15, sizeof(d3d12Options15)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS15.");
	
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &d3d12Options16, sizeof(d3d12Options16)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS16.");
}
#endif
