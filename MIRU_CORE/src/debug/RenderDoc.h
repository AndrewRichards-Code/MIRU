#pragma once
#include "GraphicsDebugger.h"
#include "renderdoc/Include/renderdoc_app.h"

#if defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

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

		//Members
	public:
		RENDERDOC_API_1_4_0* m_RenderDocApi;
	
	private:
		//RenderDoc Library
		static HMODULE s_HModuleRenderDoc;
		static std::filesystem::path s_RenderDocFullpath;
		static uint32_t s_RefCount;
	};
}
}
#elif defined(__ANDROID__)

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

		//Members
	public:
		RENDERDOC_API_1_4_0* m_RenderDocApi;

	private:
		//RenderDoc Library
		static void* s_LibRenderDoc;
		static std::string s_RenderDocFullpath;
		static uint32_t s_RefCount;
	};
}
}
#endif