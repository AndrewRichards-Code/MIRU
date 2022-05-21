#include "miru_core_common.h"
#include "Pix.h"

#if defined(_WIN64)
#include <ShlObj.h>
#endif

using namespace miru;
using namespace debug;

arc::DynamicLibrary::LibraryHandle Pix::s_WinPixEventRuntimeHandle;
std::filesystem::path Pix::s_WinPixEventRuntimeFullpath;
uint32_t Pix::s_WinPixEventRuntimeRefCount;

arc::DynamicLibrary::LibraryHandle Pix::s_WinPixGpuCapturerHandle;
std::filesystem::path Pix::s_WinPixGpuCapturerFullpath;
uint32_t Pix::s_WinPixGpuCapturerRefCount;

#if defined(_WIN64)

Pix::Pix()
{
	m_Debugger = DebuggerType::PIX;
	LoadWinPixEventRuntime();
	LoadWinPixGpuCapturer();
}

Pix::~Pix()
{
	UnloadWinPixGpuCapturer();
	UnloadWinPixEventRuntime();
}

void Pix::LoadWinPixEventRuntime()
{
	if (!s_WinPixEventRuntimeHandle)
	{
	#if defined (MIRU_WIN64_UWP)
		s_WinPixEventRuntimeFullpath = std::string(PROJECT_DIR) + "../External/pix/lib/x64/WinPixEventRuntime_UAP.dll";
	#else
		s_WinPixEventRuntimeFullpath = std::string(PROJECT_DIR) + "../External/pix/lib/x64/WinPixEventRuntime.dll";
	#endif
		s_WinPixEventRuntimeHandle = arc::DynamicLibrary::Load(s_WinPixEventRuntimeFullpath.generic_string());
		if (!s_WinPixEventRuntimeHandle)
		{
			std::string error_str = "WARN: BASE: Unable to load '" + s_WinPixEventRuntimeFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
		else
		{
			d3d12::PIXBeginEventOnCommandList = (d3d12::PFN_PIXBeginEventOnCommandList)arc::DynamicLibrary::LoadFunction(s_WinPixEventRuntimeHandle, "PIXBeginEventOnCommandList");
			d3d12::PIXEndEventOnCommandList = (d3d12::PFN_PIXEndEventOnCommandList)arc::DynamicLibrary::LoadFunction(s_WinPixEventRuntimeHandle, "PIXEndEventOnCommandList");
		}
	}
	s_WinPixEventRuntimeRefCount++;
}

void Pix::UnloadWinPixEventRuntime()
{
	s_WinPixEventRuntimeRefCount--;
	if (!s_WinPixEventRuntimeRefCount)
	{
		if (!arc::DynamicLibrary::Unload(s_WinPixEventRuntimeHandle))
		{
			std::string error_str = "WARN: BASE: Unable to free '" + s_WinPixEventRuntimeFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
}

void Pix::LoadWinPixGpuCapturer()
{
#if !defined (MIRU_WIN64_UWP)
	if (!s_WinPixGpuCapturerHandle)
	{
		LPWSTR programFilesPath;
		SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, 0, &programFilesPath);
		s_WinPixGpuCapturerFullpath = arc::ToString(programFilesPath);
		s_WinPixGpuCapturerFullpath /= "Microsoft PIX";
		std::filesystem::path pixVersionFolder;
		for (auto const& directory : std::filesystem::directory_iterator(s_WinPixGpuCapturerFullpath))
		{
			if (directory.is_directory())
			{
				if (pixVersionFolder.empty())
				{
					pixVersionFolder = directory.path();
					continue;
				}

				float dirPixVersion = std::stof(directory.path().lexically_relative(s_WinPixGpuCapturerFullpath));
				float curPixVersion = std::stof(pixVersionFolder.lexically_relative(s_WinPixGpuCapturerFullpath));
				if (dirPixVersion > curPixVersion)
				{
					pixVersionFolder = directory.path();
				}
			}
		}
		s_WinPixGpuCapturerFullpath /= pixVersionFolder / "WinPixGpuCapturer.dll";
		s_WinPixGpuCapturerHandle = arc::DynamicLibrary::Load(s_WinPixGpuCapturerFullpath.generic_string());
		if (!s_WinPixGpuCapturerHandle)
		{
			std::string error_str = "WARN: BASE: Unable to load '" + s_WinPixGpuCapturerFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
	s_WinPixGpuCapturerRefCount++;
#endif
}

void Pix::UnloadWinPixGpuCapturer()
{
#if !defined (MIRU_WIN64_UWP)
	s_WinPixGpuCapturerRefCount--;
	if (!s_WinPixGpuCapturerRefCount)
	{
		if (!arc::DynamicLibrary::Unload(s_WinPixGpuCapturerHandle))
		{
			std::string error_str = "WARN: BASE: Unable to free '" + s_WinPixGpuCapturerFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}
#endif
}

#elif defined(__ANDROID__)

Pix::Pix()
{
	m_Debugger = DebuggerType::PIX;
}

Pix::~Pix()
{
}

void Pix::LoadWinPixEventRuntime()
{
}

void Pix::UnloadWinPixEventRuntime()
{
}

void Pix::LoadWinPixGpuCapturer()
{
}

void Pix::UnloadWinPixGpuCapturer()
{
}

#endif