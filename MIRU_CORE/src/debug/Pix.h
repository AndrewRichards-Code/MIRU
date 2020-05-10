#pragma once
#include "GraphicsDebugger.h"

#if defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
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
		static HMODULE s_HModulePix;
		static std::filesystem::path s_PixFullpath;
		static uint32_t s_RefCount;
	};
}
}
#elif defined(__ANDROID__)

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
		static void* s_LibPix;
		static std::string s_PixFullpath;
		static uint32_t s_RefCount;
	};
}
}
#endif
