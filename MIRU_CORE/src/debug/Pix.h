#pragma once
#include "ARC/src/DynamicLibrary.h"
#include "GraphicsDebugger.h"

#define USE_PIX
#include "pix/Include/WinPixEventRuntime/pix3.h"

namespace miru
{
namespace debug
{
	class Pix final : public GraphicsDebugger
	{
		//Methods
	public:
		Pix();
		~Pix();

		//Members
	public:

	private:
		//Pix Libraries
		static arc::DynamicLibrary::LibraryHandle s_WinPixEventRuntimeHandle;
		static std::filesystem::path s_WinPixEventRuntimeFullpath;
		static uint32_t s_WinPixEventRuntimeRefCount;

		static arc::DynamicLibrary::LibraryHandle s_WinPixGpuCapturerHandle;
		static std::filesystem::path s_WinPixGpuCapturerFullpath;
		static uint32_t s_WinPixGpuCapturerRefCount;
	};
}
}
