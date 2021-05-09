#include "miru_core_common.h"
#include "RenderDoc.h"

using namespace miru;
using namespace debug;

arc::DynamicLibrary::LibraryHandle RenderDoc::s_RenderDocHandle;
std::filesystem::path RenderDoc::s_RenderDocFullpath;
uint32_t RenderDoc::s_RefCount = 0;

RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr)
{
	m_Debugger = DebuggerType::RENDER_DOC;
	if (!s_RenderDocHandle)
	{

#if defined(_WIN64)
		s_RenderDocFullpath = std::string(PROJECT_DIR) + "redist/renderdoc/lib/x64/renderdoc.dll";
#elif defined(__ANDROID__)
		s_RenderDocFullpath = "libVkLayer_GLES_RenderDoc.so"; 
#endif
		s_RenderDocHandle = arc::DynamicLibrary::Load(s_RenderDocFullpath.generic_string().c_str());
		if (!s_RenderDocHandle)
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_RenderDocFullpath.generic_string() + "'.";

#if defined(_WIN64)
			MIRU_WARN(GetLastError(), error_str.c_str());
#elif defined(__ANDROID__)
			MIRU_WARN(true, error_str.c_str());
#endif
			return;
		}
	}
	s_RefCount++;
	
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)arc::DynamicLibrary::LoadFunction(s_RenderDocHandle, "RENDERDOC_GetAPI");
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
		if (!arc::DynamicLibrary::Unload(s_RenderDocHandle))
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to free'" + s_RenderDocFullpath.generic_string() + "'.";

#if defined(_WIN64)
			MIRU_WARN(GetLastError(), error_str.c_str());
#elif defined(__ANDROID__)
			MIRU_WARN(true, error_str.c_str());
#endif
		}
	}
}