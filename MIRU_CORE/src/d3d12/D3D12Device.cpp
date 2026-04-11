#include "D3D12Instance.h"
#include "D3D12PhysicalDevice.h"
#include "D3D12Device.h"
#include "D3D12Sync.h"

using namespace miru;
using namespace d3d12;

Device::Device(Device::CreateInfo* pCreateInfo)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	InstanceRef instance = ref_cast<Instance>(GetInstance());
	PhysicalDeviceRef physicalDevice = ref_cast<PhysicalDevice>(m_CI.physicalDevice);
	IDXGIAdapter4*& adapter = physicalDevice->m_Adapter;

	//Check provided feature level
	D3D_FEATURE_LEVEL featureLevel;
	for (size_t i = 0; i < std::size(m_Features.featureLevelsList); i++)
	{
		featureLevel = m_Features.featureLevelsList[i];
		HRESULT res = D3D12CreateDevice(adapter, featureLevel, __uuidof(ID3D12Device), nullptr);
		if (res == S_FALSE)
			break;
		else
			continue;
	}

	//OpenXR Data
	Instance::OpenXRD3D12Data* openXRD3D12Data = reinterpret_cast<Instance::OpenXRD3D12Data*>(instance->GetCreateInfo().pNext);
	if (!(openXRD3D12Data && openXRD3D12Data->type == Instance::CreateInfoExtensionStructureTypes::OPENXR_D3D12_DATA))
		openXRD3D12Data = nullptr;

	if (openXRD3D12Data && featureLevel < openXRD3D12Data->minFeatureLevel)
	{
		MIRU_FATAL(true, "ERROR: D3D12: Selected D3D_FEATURE_LEVEL is less than the minimum for OpenXR.");
	}

	//Create Device
	MIRU_FATAL(D3D12CreateDevice(adapter, featureLevel, IID_PPV_ARGS(&m_Device)), "ERROR: D3D12: Failed to create Device.");
	D3D12SetName(m_Device, m_CI.debugName);

	m_RI.apiVersionMajor = (((uint32_t)(featureLevel) >> 12) & 0xFU);
	m_RI.apiVersionMinor = (((uint32_t)(featureLevel) >> 8) & 0xFU);
	m_RI.apiVersionPatch = 0;

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
	if (m_Features.d3d12Options4.Native16BitShaderOpsSupported)
		m_RI.activeExtensions |= ExtensionsBit::SHADER_NATIVE_16_BIT_TYPES;

	m_RI.activeExtensions &= m_CI.extensions;
	m_RI.deviceName = arc::ToString(physicalDevice->m_AdapterDesc.Description);

	//Create Info Queue
	if (m_CI.debugValidationLayers)
	{
		m_Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue));
		if (m_InfoQueue)
		{
			m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, false);
			m_InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_MESSAGE, false);

			D3D12_MESSAGE_ID filteredMessageIDs[2] = { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE };
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.pIDList = filteredMessageIDs;
			filter.DenyList.NumIDs = std::size(filteredMessageIDs);
			m_InfoQueue->AddStorageFilterEntries(&filter);

			reinterpret_cast<ID3D12InfoQueue1*>(m_InfoQueue)->RegisterMessageCallback(MessageCallbackFunction, D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &m_CallbackCookie);
		}
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
		MIRU_FATAL(m_Device->CreateCommandQueue(&m_QueueDescs[i], IID_PPV_ARGS(&m_Queues[i])), "ERROR: D3D12: Failed to create CommandQueue.");

		std::string typeStr = i == 0 ? "Direct" : i == 1 ? "Compute" : i == 2 ? "Copy" : "";
		D3D12SetName(m_Queues[i], m_CI.debugName + ": Queue - " + typeStr);
	}
}

Device::~Device()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (m_InfoQueue)
		reinterpret_cast<ID3D12InfoQueue1*>(m_InfoQueue)->UnregisterMessageCallback(m_CallbackCookie);

	MIRU_D3D12_SAFE_RELEASE(m_InfoQueue);

	ID3D12DebugDevice* debugDevice = nullptr;
	m_Device->QueryInterface(&debugDevice);

	for (auto& queue : m_Queues)
		MIRU_D3D12_SAFE_RELEASE(queue);

	MIRU_D3D12_SAFE_RELEASE(m_Device);

	if (debugDevice)
	{
		debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_IGNORE_INTERNAL);
		debugDevice->Release();
	}
}

void Device::DeviceWaitIdle()
{
	MIRU_CPU_PROFILE_FUNCTION();

	base::FenceRef fence;
	Fence::CreateInfo ci;
	ci.debugName = "";
	ci.device = DeviceRef(DeviceRef(), this); //Aliasing constructor: https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
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

void Device::MessageCallbackFunction(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext)
{
	std::string category = std::string(magic_enum::enum_name<D3D12_MESSAGE_CATEGORY>(Category));
	std::string severity = std::string(magic_enum::enum_name<D3D12_MESSAGE_SEVERITY>(Severity));
	//std::string id = std::string(magic_enum::enum_name<D3D12_MESSAGE_ID>(ID));
	std::string description = pDescription;

	category = category.substr(std::string("D3D12_MESSAGE_CATEGORY_").size());
	severity = severity.substr(std::string("D3D12_MESSAGE_SEVERITY_").size());

	std::stringstream errorMessage;
	errorMessage << "D3D12 " << severity << ": " << description << " [ " << category << " " << severity << " #" << uint32_t(ID) << ": " /*<< id*/ << " ]";
	std::string errorMessageStr = errorMessage.str();

	switch (Severity)
	{
	case D3D12_MESSAGE_SEVERITY_CORRUPTION:
	{
		MIRU_FATAL(uint32_t(ID), errorMessageStr.c_str());
		ARC_DEBUG_BREAK;
		break;
	}
	case D3D12_MESSAGE_SEVERITY_ERROR:
	{
		MIRU_ERROR(uint32_t(ID), errorMessageStr.c_str());
		ARC_DEBUG_BREAK;
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

Device::Features::Features(ID3D12Device* device)
{
	MIRU_CPU_PROFILE_FUNCTION();

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12Options, sizeof(d3d12Options)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS.");

	featureLevels.NumFeatureLevels = std::size(featureLevelsList);
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

	shaderModel.HighestShaderModel = D3D_HIGHEST_SHADER_MODEL;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_MODEL.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &d3d12Options1, sizeof(d3d12Options1)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS1.");

	protectedResourceSessionSupport.NodeIndex = 0;
	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, &protectedResourceSessionSupport, sizeof(protectedResourceSessionSupport)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT.");

	rootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_2;
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

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS17, &d3d12Options17, sizeof(d3d12Options17)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS17.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS18, &d3d12Options18, sizeof(d3d12Options18)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS18.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, &d3d12Options19, sizeof(d3d12Options19)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS19.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS20, &d3d12Options20, sizeof(d3d12Options20)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS20.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &d3d12Options21, sizeof(d3d12Options21)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS21.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS20, &d3d12Options20, sizeof(d3d12Options20)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS20.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_PREDICATION, &predication, sizeof(predication)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_PREDICATION.");

	for (uint32_t i = 0; i <= uint32_t(DXGI_FORMAT_B4G4R4A4_UNORM); i++)
	{
		std::vector<std::vector<D3D12_FEATURE_DATA_PLACED_RESOURCE_SUPPORT_INFO>> _placedResourceSupportInfo;
		for (uint32_t j = 0; j <= uint32_t(D3D12_RESOURCE_DIMENSION_TEXTURE3D); j++)
		{
			std::vector<D3D12_FEATURE_DATA_PLACED_RESOURCE_SUPPORT_INFO> __placedResourceSupportInfo;
			for (uint32_t k = 0; k <= uint32_t(D3D12_HEAP_TYPE_GPU_UPLOAD); k++)
			{
				D3D12_FEATURE_DATA_PLACED_RESOURCE_SUPPORT_INFO placedResourceSupportInfo = { DXGI_FORMAT(i), D3D12_RESOURCE_DIMENSION(j), { D3D12_HEAP_TYPE(k), D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 } };
				MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_PLACED_RESOURCE_SUPPORT_INFO, &placedResourceSupportInfo, sizeof(placedResourceSupportInfo)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_PLACED_RESOURCE_SUPPORT_INFO.");
				__placedResourceSupportInfo.push_back(placedResourceSupportInfo);
			}
			_placedResourceSupportInfo.push_back(__placedResourceSupportInfo);
		}
		placedResourceSupportInfos.push_back(_placedResourceSupportInfo);
	}

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_HARDWARE_COPY, &hardwareCopy, sizeof(hardwareCopy)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_HARDWARE_COPY.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &d3d12Options21, sizeof(d3d12Options21)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS21.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_TIGHT_ALIGNMENT, &tightAlignment, sizeof(tightAlignment)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_TIGHT_ALIGNMENT.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_APPLICATION_SPECIFIC_DRIVER_STATE, &applicationSpecificDriverState, sizeof(applicationSpecificDriverState)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_APPLICATION_SPECIFIC_DRIVER_STATE.");

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_BYTECODE_BYPASS_HASH_SUPPORTED, &bytecodeBypassHashSupported, sizeof(bytecodeBypassHashSupported)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_BYTECODE_BYPASS_HASH_SUPPORTED.");

	for (uint32_t i = 0; i <= D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE; i++)
	{
		std::vector<D3D12_FEATURE_DATA_BARRIER_LAYOUT> _barrierLayouts;
		for (uint32_t j = 0; j <= D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ_COMPUTE_QUEUE_ACCESSIBLE; j++)
		{
			D3D12_FEATURE_DATA_BARRIER_LAYOUT barrierLayout = { D3D12_COMMAND_LIST_TYPE(i), D3D12_BARRIER_LAYOUT(j), false };
			MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_BARRIER_LAYOUT, &barrierLayout, sizeof(barrierLayout)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_BARRIER_LAYOUT.");
			_barrierLayouts.push_back(barrierLayout);
		}
		barrierLayouts.push_back(_barrierLayouts);
	}

	MIRU_WARN(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS22, &d3d12Options22, sizeof(d3d12Options22)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_D3D12_OPTIONS22.");
}