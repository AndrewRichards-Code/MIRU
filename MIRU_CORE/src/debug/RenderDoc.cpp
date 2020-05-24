#include "miru_core_common.h"
#include "RenderDoc.h"

using namespace miru;
using namespace debug;

#if defined(_WIN64)
HMODULE RenderDoc::s_HModuleRenderDoc;
std::filesystem::path RenderDoc::s_RenderDocFullpath;
uint32_t RenderDoc::s_RefCount = 0;

RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr)
{
	m_Debugger = DebuggerType::RENDER_DOC;
	if (!s_HModuleRenderDoc)
	{
		s_RenderDocFullpath = std::string(SOLUTION_DIR) + "MIRU_CORE/redist/renderdoc/lib/x64/renderdoc.dll";
		s_HModuleRenderDoc = LoadLibraryA(s_RenderDocFullpath.generic_string().c_str());
		if (!s_HModuleRenderDoc)
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_RenderDocFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
			return;
		}
	}
	s_RefCount++;
	
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(s_HModuleRenderDoc, "RENDERDOC_GetAPI");
	if (!RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, (void**)&m_RenderDocApi))
	{
		std::string error_str = "WARN: CROSSPLATFORM: Unable to initial RenderDoc.";
		MIRU_WARN(true, error_str.c_str());
		return;
	}
}

RenderDoc::~RenderDoc()
{
	s_RefCount--;
	if (!s_RefCount)
	{
		if (!FreeLibrary(s_HModuleRenderDoc))
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to free'" + s_RenderDocFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
}

#elif defined(__ANDROID__)

void* RenderDoc::s_LibRenderDoc;
std::string RenderDoc::s_RenderDocFullpath;
uint32_t RenderDoc::s_RefCount = 0;

RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr)
{
	m_Debugger = DebuggerType::RENDER_DOC;
	if (!s_LibRenderDoc)
	{
		s_RenderDocFullpath = "libVkLayer_GLES_RenderDoc.so";
		s_LibRenderDoc = dlopen(s_RenderDocFullpath.c_str(), RTLD_NOW | RTLD_NOLOAD);
		if (!s_LibRenderDoc)
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_RenderDocFullpath + "'.";
			MIRU_WARN(true, error_str.c_str());
			return;
		}
	}
	s_RefCount++;
	
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(s_LibRenderDoc, "RENDERDOC_GetAPI");
	if (!RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, (void**)&m_RenderDocApi))
	{
		std::string error_str = "WARN: CROSSPLATFORM: Unable to initial RenderDoc.";
		MIRU_WARN(true, error_str.c_str());
		return;
	}
}

RenderDoc::~RenderDoc()
{
	s_RefCount--;
	if (!s_RefCount)
	{
		if (!dlclose(s_LibRenderDoc))
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to free'" + s_RenderDocFullpath + "'.";
			MIRU_WARN(true, error_str.c_str());
		}
	}
}

#endif