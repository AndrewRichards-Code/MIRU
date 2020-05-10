#pragma once
#include "debug/Pix.h"
#include "debug/RenderDoc.h"

namespace miru
{
	class GraphicsAPI
	{
	public:
		enum class API : uint32_t
		{
			UNKNOWN = 0,
			D3D12,
			VULKAN
		};

	public:
		GraphicsAPI() = delete;
		~GraphicsAPI() = delete;

		static void SetAPI(API api);
		static const API& GetAPI() { return s_API; }

		static bool IsD3D12() { return s_API == API::D3D12; }
		static bool IsVulkan() { return s_API == API::VULKAN; }

		static void AllowSetName(bool useSetName = true);
		static const bool& IsSetNameAllowed() { return s_AllowSetName; }
		
		static void LoadGraphicsDebugger();
		template<class T>
		static const std::shared_ptr<T>& GetGraphicsDebugger() { return std::dynamic_pointer_cast<T>(s_GraphicsDebugger);}
		static const std::shared_ptr<debug::GraphicsDebugger>& GetGraphicsDebugger() { return s_GraphicsDebugger;}

	private:
		static API s_API;
		static bool s_AllowSetName;

		static bool s_ApiInitialised;
		static bool s_AllowSetNameInitialised;

		static std::shared_ptr<debug::GraphicsDebugger> s_GraphicsDebugger;
	};
}