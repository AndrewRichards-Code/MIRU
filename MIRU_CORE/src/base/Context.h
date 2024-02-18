#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace base
{
	class MIRU_API Context
	{
		//enums/structs
	public:
		enum class ExtensionsBit : uint32_t
		{
			NONE						= 0x00000000,
			
			//STATUS: O
			//D3D12: https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
			//Vulkan: VK_KHR_ray_tracing_pipeline, VK_KHR_ray_query and VK_KHR_acceleration_structure: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap38.html#ray-tracing
			RAY_TRACING					= 0x00000001,
			
			//STATUS: X
			//D3D12: Core: https://docs.microsoft.com/en-us/windows/win32/direct3d12/example-root-signatures#streaming-shader-resource-views
			//Vulkan: VK_EXT_descriptor_indexing: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_EXT_descriptor_indexing
			DESCRIPTOR_INDEXING			= 0x00000002,
			
			//STATUS: O
			//D3D12: Core
			//Vulkan: VK_KHR_timeline_semaphore: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_KHR_timeline_semaphore
			TIMELINE_SEMAPHORE			= 0x00000004,
			
			//STATUS: O - Excluding Vulkan Events2.
			//D3D12: Enhanced Barriers
			//Vulkan: VK_KHR_synchronization2: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_KHR_synchronization2
			SYNCHRONISATION_2			= 0x00000008,

			//STATUS: X
			//D3D12: https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html
			//Vulkan: VK_EXT_mesh_shader: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap52.html#VK_EXT_mesh_shader
			MESH_SHADER					= 0x00000010,
			
			//STATUS: X
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/direct3d12/vrs
			//Vulkan: VK_KHR_fragment_shading_rate: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_KHR_fragment_shading_rate
			FRAGMENT_SHADING_RATE		= 0x00000020,
			
			//STATUS: O
			//D3D12: Core
			//Vulkan: VK_KHR_dynamic_rendering: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_KHR_dynamic_rendering
			DYNAMIC_RENDERING			= 0x00000040,
			
			//STATUS: X
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/direct3d12/conservative-rasterization
			//Vulkan: VK_EXT_conservative_rasterization: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_EXT_conservative_rasterization
			CONSERVATIVE_RASTERISATION	= 0x00000080,
			
			//STATUS: X
			//D3D12: https://docs.microsoft.com/en-gb/windows/win32/direct3d12/predication
			//Vulkan: VK_EXT_conditional_rendering: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_EXT_conditional_rendering
			CONDITIONAL_RENDERING		= 0x00000100,

			//STATUS: O
			//D3D12: https://microsoft.github.io/DirectX-Specs/d3d/ViewInstancing.html
			//Vulkan: VK_KHR_multiview : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#VK_KHR_multiview 
			MULTIVIEW					= 0x00000200,

			//STATUS: O		Allows SV_RenderTargetArrayIndex/gl_Layer and SV_ViewportArrayIndex/gl_ViewportIndex in pre-rasterisation shaders
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
			//Vulkan: VK_EXT_shader_viewport_index_layer : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VK_EXT_shader_viewport_index_layer
			SHADER_VIEWPORT_INDEX_LAYER = 0x00000400,

			//STATUS: O		Allows the use of int16, uint16 and float16 in shaders
			//D3D12: https://github.com/MicrosoftDocs/sdk-api/blob/docs/sdk-api-src/content/d3d12/ns-d3d12-d3d12_feature_data_d3d12_options4.md, https://github.com/microsoft/DirectXShaderCompiler/wiki/16-Bit-Scalar-Types
			//Vulkan: VK_KHR_shader_float16_int8, VK_KHR_16bit_storage : https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_shader_float16_int8.html, https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_16bit_storage.html
			SHADER_NATIVE_16_BIT_TYPES	 = 0x00000800,

			//STATUS: X 
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/medfound/direct3d-12-video-overview
			//Vulkan: VK_KHR_video_queue, VK_KHR_video_encode_queue, VK_KHR_video_encode_h264/_h265 : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#provisional-extension-appendices-list
			VIDEO_ENCODE				= 0x00010000,
			
			//STATUS: X 
			//D3D12: https://docs.microsoft.com/en-us/windows/win32/medfound/direct3d-12-video-overview
			//| Vulkan: VK_KHR_video_queue, VK_KHR_video_decode_queue, VK_KHR_video_decode_h264/_h265 : https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap52.html#provisional-extension-appendices-list
			VIDEO_DECODE				= 0x00020000,
		};
		struct CreateInfo
		{
			std::string		applicationName;
			bool			debugValidationLayers;
			ExtensionsBit	extensions;
			std::string		deviceDebugName;
			void*			pNext;
		};
		struct ResultInfo
		{
			uint32_t		apiVersionMajor;
			uint32_t		apiVersionMinor;
			uint32_t		apiVersionPatch;
			ExtensionsBit	activeExtensions;
			std::string		deviceName;
		};
		enum class CreateInfoExtensionStructureTypes : uint32_t
		{
			UNKNOWN,
			OPENXR_D3D12_DATA,
			OPENXR_VULKAN_DATA,
		};

		//Methods
	public:
		static ContextRef Create(CreateInfo* pCreateInfo);
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