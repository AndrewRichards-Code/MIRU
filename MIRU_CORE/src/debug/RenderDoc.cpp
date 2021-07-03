#include "miru_core_common.h"
#include "Pix.h"
#include "RenderDoc.h"

#if defined(_WIN64)
#include <ShlObj.h>
#endif

using namespace miru;
using namespace debug;

arc::DynamicLibrary::LibraryHandle RenderDoc::s_RenderDocHandle;
std::filesystem::path RenderDoc::s_RenderDocFullpath;
uint32_t RenderDoc::s_RefCount = 0;

RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr)
{
	m_Debugger = DebuggerType::RENDER_DOC;
	LoadRenderDoc();
	if (crossplatform::GraphicsAPI::IsD3D12())
		Pix::LoadWinPixEventRuntime();
}

RenderDoc::~RenderDoc()
{
	UnloadRenderDoc();
	if (crossplatform::GraphicsAPI::IsD3D12())
		Pix::UnloadWinPixEventRuntime();
}

void RenderDoc::LoadRenderDoc()
{
	if (!s_RenderDocHandle)
	{
	#if defined(_WIN64)
		LPWSTR programFilesPath;
		SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, 0, &programFilesPath);
		s_RenderDocFullpath = arc::ToString(programFilesPath);
		s_RenderDocFullpath /= "RenderDoc";
		s_RenderDocFullpath /= "renderdoc.dll";
	#elif defined(__ANDROID__)
		s_RenderDocFullpath = "libVkLayer_GLES_RenderDoc.so"; 
	#endif
		s_RenderDocHandle = arc::DynamicLibrary::Load(s_RenderDocFullpath.generic_string());
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

void RenderDoc::UnloadRenderDoc()
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