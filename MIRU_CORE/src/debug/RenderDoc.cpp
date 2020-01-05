#include "common.h"
#include "RenderDoc.h"

using namespace miru;

#if defined(_WIN64)
#include <Windows.h>

RenderDoc::RenderDoc()
	:m_RenderDocApi(nullptr), m_HInstance(nullptr), m_HModule(nullptr)
{
	if (m_HInstance = LoadLibrary(MIRU_RENDERDOC_LIBRARY))
	{
		if (m_HModule = GetModuleHandleA(MIRU_RENDERDOC_LIBRARY))
		{
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(m_HModule, "RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, (void**)&m_RenderDocApi);
		}
	}
}

#else

RenderDoc: RenderDoc()
	:m_RenderDocApi(nullptr), m_HInstance(nullptr), m_HModule(nullptr)
{
}

#endif