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
		//Pix Library
		static arc::DynamicLibrary::LibraryHandle s_PixHandle;
		static std::filesystem::path s_PixFullpath;
		static uint32_t s_RefCount;
	};
}
}
