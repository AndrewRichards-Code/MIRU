#pragma once
#include "renderdoc/Include/renderdoc_app.h"

#if defined(_WIN64)
#include <Windows.h>

namespace miru
{
namespace debug
{
	class RenderDoc
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
#else

namespace miru
{
namespace debug
{
	class RenderDoc
	{
		//Methods
	public:
		RenderDoc();

		//Members
	public:
		RENDERDOC_API_1_4_0* m_RenderDocApi;

	private:
		void* m_Instance;
		void* m_Module;
	};
}
}
#endif