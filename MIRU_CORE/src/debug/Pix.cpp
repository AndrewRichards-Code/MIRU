#include "miru_core_common.h"
#include "Pix.h"

using namespace miru;
using namespace debug;

arc::DynamicLibrary::LibraryHandle Pix::s_PixHandle;
std::filesystem::path Pix::s_PixFullpath;
uint32_t Pix::s_RefCount = 0;

#if defined(_WIN64)

Pix::Pix()
{
	m_Debugger = DebuggerType::PIX;
	if (!s_PixHandle)
	{
		s_PixFullpath = std::string(PROJECT_DIR) + "redist/pix/lib/x64/WinPixEventRuntime.dll";
		s_PixHandle = arc::DynamicLibrary::Load(s_PixFullpath.generic_string().c_str());
		if (!s_PixHandle)
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
		if (!arc::DynamicLibrary::Unload(s_PixHandle))
		{
			std::string error_str = "WARN: CROSSPLATFORM: Unable to free '" + s_PixFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
}

#elif defined(__ANDROID__)

Pix::Pix()
{
	m_Debugger = DebuggerType::PIX;
}

Pix::~Pix()
{
}

#endif