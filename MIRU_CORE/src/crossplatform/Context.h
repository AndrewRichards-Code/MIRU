#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace crossplatform
{
	class MIRU_API Context
	{
		//enums/structs
	public:
		enum class ExtensionsBit : uint32_t
		{
			NONE					= 0x00000000,
			//STATUS: O
			//D3D12: https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
			//Vulkan: VK_KHR_ray_tracing_pipeline and VK_KHR_acceleration_structure: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap38.html#ray-tracing
			RAY_TRACING				= 0x00000001,
			//STATUS: X
			//D3D12: Core: https://docs.microsoft.com/en-us/windows/win32/direct3d12/example-root-signatures#streaming-shader-resource-views
			//Vulkan: VK_EXT_descriptor_indexing: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#VK_EXT_descriptor_indexing
			DESCRIPTOR_INDEXING		= 0x00000002,
			//STATUS: X
			//D3D12: Core
			//Vulkan: VK_KHR_timeline_semaphore: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#VK_KHR_timeline_semaphore
			TIMELINE_SEMAPHORE		= 0x00000004,
			//STATUS: X
			//D3D12: https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html
			//Vulkan: VK_NV_mesh_shader: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#VK_NV_mesh_shader
			MESH_SHADER_NV			= 0x00000008,
			//STATUS: X
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/direct3d12/vrs
			//Vulkan: VK_KHR_fragment_shading_rate: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#VK_KHR_fragment_shading_rate
			FRAGMENT_SHADING_RATE	= 0x00000010,
			//STATUS: O
			//D3D12: Core
			//Vulkan: VK_KHR_dynamic_rendering: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#VK_KHR_dynamic_rendering
			DYNAMIC_RENDERING		= 0x00000020,
			//STATUS: X 
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/medfound/direct3d-12-video-overview
			//Vulkan: VK_KHR_video_queue, VK_KHR_video_encode_queue, VK_KHR_video_encode_h264/_h265 : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#provisional-extension-appendices-list
			VIDEO_ENCODE			= 0x00000040,
			//STATUS: X 
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/medfound/direct3d-12-video-overview
			//| Vulkan: VK_KHR_video_queue, VK_KHR_video_decode_queue, VK_KHR_video_decode_h264/_h265 : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap50.html#provisional-extension-appendices-list
			VIDEO_DECODE			= 0x00000080,
		};
		struct CreateInfo
		{
			std::string		applicationName;
			bool			debugValidationLayers;
			ExtensionsBit	extensions;
			std::string		deviceDebugName;
		};
		struct ResultInfo
		{
			uint32_t		apiVersionMajor;
			uint32_t		apiVersionMinor;
			uint32_t		apiVersionPatch;
			ExtensionsBit	activeExtensions;
		};

		//Methods
	public:
		static Ref<Context> Create(CreateInfo* pCreateInfo);
		virtual ~Context() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }
		const ResultInfo& GetResultInfo() { return m_RI; }

		virtual void* GetDevice() = 0;
		virtual void DeviceWaitIdle() = 0;

		//Members
	protected:
		CreateInfo m_CI = {};
		ResultInfo m_RI = {};
	};
}
}