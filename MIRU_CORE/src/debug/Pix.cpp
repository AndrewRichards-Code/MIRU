#include "miru_core_common.h"
#include "Pix.h"

using namespace miru;
using namespace debug;

#if defined(_WIN64)
HMODULE Pix::s_HModulePix;
std::filesystem::path Pix::s_PixFullpath;
uint32_t Pix::s_RefCount = 0;

Pix::Pix()
{
	m_Debugger = DebuggerType::PIX;
	if (!s_HModulePix)
	{
		s_PixFullpath = std::string(SOLUTION_DIR) + "MIRU_CORE/redist/pix/lib/x64/WinPixEventRuntime.dll";
		s_HModulePix = LoadLibraryA(s_PixFullpath.generic_string().c_str());
		if (!s_HModulePix)
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_PixFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
}

Pix::~Pix()
{
	s_RefCount--;
	if (!s_RefCount)
	{
		if (!FreeLibrary(s_HModulePix))
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to free '" + s_PixFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
}

#elif defined(__ANDROID__)

void* Pix::s_LibPix;
std::string Pix::s_PixFullpath;
uint32_t Pix::s_RefCount = 0;

Pix::Pix()
{
	m_Debugger = DebuggerType::PIX;
}

Pix::~Pix()
{
}

#endif