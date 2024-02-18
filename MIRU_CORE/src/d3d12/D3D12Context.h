#pragma once
#include "base/Context.h"
#include "d3d12/D3D12_Include.h"
#include <filesystem>

namespace miru
{
namespace d3d12
{
	class Context final : public base::Context
	{
		//enum/structs
	public:
		struct PhysicalDevices
		{
			struct PhysicalDeviceInfo
			{
				IDXGIAdapter4* m_Adapter;
				DXGI_ADAPTER_DESC m_AdapterDesc;
			};
			std::vector<PhysicalDeviceInfo> m_PDIs;

			PhysicalDevices(IDXGIFactory4* factory);
			PhysicalDevices() {};
			PhysicalDevices(const PhysicalDevices& physicalDevice)
			{
				for (const PhysicalDeviceInfo& physicalDeviceInfo : physicalDevice.m_PDIs)
				{
					m_PDIs.push_back(physicalDeviceInfo);
				}
			};
		};

		struct Features
		{
			D3D12_FEATURE_DATA_D3D12_OPTIONS										d3d12Options = {};
			D3D12_FEATURE_DATA_FEATURE_LEVELS										featureLevels = {};
			std::vector<D3D12_FEATURE_DATA_FORMAT_SUPPORT>							formatSupports;
			std::vector<std::vector<D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS>>	multisampleQualityLevels = {};
			std::vector<D3D12_FEATURE_DATA_FORMAT_INFO>								formatInfos;
			D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT							gpuVirtualAddressSupport = {};
			D3D12_FEATURE_DATA_SHADER_MODEL											shaderModel = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS1										d3d12Options1 = {};
			D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT					protectedResourceSessionSupport = {};
			D3D12_FEATURE_DATA_ROOT_SIGNATURE										rootSignature = {};
			D3D12_FEATURE_DATA_ARCHITECTURE1										architecture1 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS2										d3d12Options2 = {};
			D3D12_FEATURE_DATA_SHADER_CACHE											shaderCache = {};
			std::vector<std::vector<D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY>>		commandQueuePriorities = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS3										d3d12Options3 = {};
			D3D12_FEATURE_DATA_EXISTING_HEAPS										existingHeaps = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS4										d3d12Options4 = {};
			D3D12_FEATURE_DATA_SERIALIZATION										serialisation = {};
			D3D12_FEATURE_DATA_CROSS_NODE											crossNode = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS5										d3d12Options5 = {};
			D3D12_FEATURE_DATA_DISPLAYABLE											displayable = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS6										d3d12Options6 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS7										d3d12Options7 = {};
			D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT				protectedResourceSessionTypeCount = {};
			D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES						protectedResourceSessionTypes = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS8										d3d12Options8 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS9										d3d12Options9 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS10										d3d12Options10 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS11										d3d12Options11 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS12										d3d12Options12 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS13										d3d12Options13 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS14										d3d12Options14 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS15										d3d12Options15 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS16										d3d12Options16 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS17										d3d12Options17 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS18										d3d12Options18 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS19										d3d12Options19 = {};

			D3D_FEATURE_LEVEL featureLevelsList[5] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
			std::vector<GUID> protectedResourceSessionTypesGuids;

			Features() = default;
			Features(ID3D12Device* device);
		};

		struct OpenXRD3D12Data
		{
			CreateInfoExtensionStructureTypes	type;
			void*								pNext;
			LUID								adapterLuid;
			D3D_FEATURE_LEVEL					minFeatureLevel;
		};

		//Methods
	public:
		Context(Context::CreateInfo* pCreateInfo);
		~Context();

		void* GetDevice() override { return m_Device; }
		void DeviceWaitIdle() override;

		static void MessageCallbackFunction(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext);

		static std::string D3D12_MESSAGE_CATEGORY_ToString(D3D12_MESSAGE_CATEGORY Category);
		static std::string D3D12_MESSAGE_SEVERITY_ToString(D3D12_MESSAGE_SEVERITY Severity);
		static std::string D3D12_MESSAGE_ID_ToString(D3D12_MESSAGE_ID ID);

		//Member
	public:
		//Debug
		ID3D12Debug* m_Debug = nullptr;
		ID3D12InfoQueue* m_InfoQueue = nullptr;
		DWORD m_CallbackCookie = 0;

		//Factory
		IDXGIFactory4* m_Factory = nullptr;

		//Device
		ID3D12Device* m_Device = nullptr;
		PhysicalDevices m_PhysicalDevices = {};
		size_t m_PhysicalDeviceIndex = 0;

		//Queue
		std::vector<ID3D12CommandQueue*> m_Queues;
		std::vector<D3D12_COMMAND_QUEUE_DESC> m_QueueDescs;

		//Features
		Features m_Features;
	};
}
}