#pragma once
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
		static void SetAPI(API api);
		static const API& GetAPI() { return s_API; }

		static void SetUseSetName(bool useSetName = true);
		static const bool& GetUseSetName() { return s_UseSetName; }
		
		static void LoadRenderDoc();
		static const debug::RenderDoc& GetRenderDoc() { return s_RenderDoc; }

	private:
		static API s_API;
		static bool s_UseSetName;

		static bool s_ApiInitialised;
		static bool s_UseSetNameInitialised;

		static debug::RenderDoc s_RenderDoc;
	};
}