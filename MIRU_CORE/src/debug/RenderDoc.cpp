#include "miru_core_common.h"
#include "RenderDoc.h"

using namespace miru;
using namespace debug;

HMODULE RenderDoc::s_HModuleRenderDoc;
std::filesystem::path RenderDoc::s_RenderDocFullpath;
uint32_t RenderDoc::s_RefCount = 0;

#if defined(_WIN64)
RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr)
{
	if (!s_HModuleRenderDoc)
	{
		s_RenderDocFullpath = std::string(SOLUTION_DIR) + "MIRU_CORE/redist/renderdoc/lib/x64/renderdoc.dll";
		s_HModuleRenderDoc = LoadLibraryA(s_RenderDocFullpath.generic_string().c_str());
		if (!s_HModuleRenderDoc)
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_RenderDocFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
	else
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(s_HModuleRenderDoc, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, (void**)&m_RenderDocApi);
	}
	s_RefCount++;
}

RenderDoc::~RenderDoc()
{
	s_RefCount--;
	if (!s_RefCount)
	{
		if (!FreeLibrary(s_HModuleRenderDoc))
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_RenderDocFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
}

#else

RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr)
{
}

#endif