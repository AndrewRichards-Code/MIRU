#pragma once
#define MIRU_RENDERDOC_HEADER "C:/Program Files/RenderDoc/renderdoc_app.h"
#define MIRU_RENDERDOC_LIBRARY "C:/Program Files/RenderDoc/renderdoc.dll"
#include MIRU_RENDERDOC_HEADER

#if defined(_WIN64)
#include <Windows.h>

namespace miru
{
	class RenderDoc
	{
		//Methods
	public:
		RenderDoc();

		//Members
	public:
		RENDERDOC_API_1_4_0* m_RenderDocApi;
	
	private:
		HINSTANCE m_HInstance;
		HMODULE m_HModule;
	};
}

#else

namespace miru
{
	class RenderDoc
	{
		//Methods
	public:
		RenderDoc();

		//Members
	public:
		RENDERDOC_API_1_4_0* m_RenderDocApi;

	private:
		void* m_Instance;
		void* m_Module;
	};
}

#endif