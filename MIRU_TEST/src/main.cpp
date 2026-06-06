#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "base/GraphicsAPI.h"

//D3D12SDK exported variables for D3D12Core.dll to pick up.
extern "C"
{
	__declspec(dllexport) extern const unsigned int D3D12SDKVersion = 619;
	__declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

#if defined(_WIN64)
//Forward Declaration from other cpp files.
void Basic(uint32_t maxFrames = UINT32_MAX);
void Raytracing(uint32_t maxFrames = UINT32_MAX);
void DynamicState(uint32_t maxFrames = UINT32_MAX);
void Multiview(uint32_t maxFrames = UINT32_MAX);
void MeshShader(uint32_t maxFrames = UINT32_MAX);
void Sync2(uint32_t maxFrames = UINT32_MAX);

#define MIRU_TEST_BASIC			1
#define MIRU_TEST_RAYTRACING	1
#define MIRU_TEST_DYNAMIC_STATE	1
#define MIRU_TEST_MULTIVIEW		1
#define MIRU_TEST_MESH_SHADER	1
#define MIRU_TEST_SYNC2			0

#define MIRU_TEST_NO_FRAME_LIMIT 0

int main()
{
	using namespace miru;
	using namespace base;

	//GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
	GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
	GraphicsAPI::AllowSetName();
	GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::NONE);

#if MIRU_TEST_NO_FRAME_LIMIT
	uint32_t maxFrames = UINT32_MAX;
#else
	uint32_t maxFrames = 120; //~2 second(s) at 60Hz.
#endif

#if MIRU_TEST_BASIC
	Basic(maxFrames);
#endif
#if MIRU_TEST_RAYTRACING
	Raytracing(maxFrames);
#endif
#if MIRU_TEST_DYNAMIC_STATE
	DynamicState(maxFrames);
#endif
#if MIRU_TEST_MULTIVIEW
	Multiview(maxFrames);
#endif
#if MIRU_TEST_MESH_SHADER
	MeshShader(maxFrames);
#endif
#if MIRU_TEST_SYNC2
	Sync2(maxFrames);
#endif

}
#elif defined(__ANDROID__)
#include "android_native_app_glue.h"

//Externs
android_app* g_App = nullptr;

//Forward Declaration from other cpp files.
void Basic();

extern "C" { void android_main(struct android_app* app); }
void android_main(struct android_app* app)
{
	GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
	GraphicsAPI::AllowSetName(false);
	GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::NONE);

	g_App = app;
	Basic();
}

#endif