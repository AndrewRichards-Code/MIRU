#pragma once
#include "GraphicsDebugger.h"
#include "ARC/src/DynamicLibrary.h"
#include "renderdoc/Include/renderdoc_app.h"
#include <filesystem>

namespace miru
{
namespace debug
{
	class RenderDoc final : public GraphicsDebugger
	{
		//Methods
	public:
		RenderDoc();
		~RenderDoc();

	private:
		void LoadRenderDoc();
		void UnloadRenderDoc();

		//Members
	public:
		RENDERDOC_API_1_4_0* m_RenderDocApi;

	private:
		//RenderDoc Library
		static arc::DynamicLibrary::LibraryHandle s_RenderDocHandle;
		static std::filesystem::path s_RenderDocFullpath;
		static uint32_t s_RefCount;
	};
}
}
