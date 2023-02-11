#define STB_IMAGE_IMPLEMENTATION
#include "STBI/stb_image.h"

#if defined(_WIN64)
//Forward Declaration from other cpp files.
void Basic();
void Raytracing();
void DynamicRendering();
void Multiview();
void MeshShader();

#define MIRU_TEST_RAYTRACING 0
#define MIRU_TEST_DYNAMIC_RENDERING 0
#define MIRU_TEST_MULTIVIEW 0
#define MIRU_TEST_MESH_SHADER 1

int main()
{
#if MIRU_TEST_RAYTRACING
	Raytracing();
#elif MIRU_TEST_DYNAMIC_RENDERING
	DynamicRendering();
#elif MIRU_TEST_MULTIVIEW
	Multiview();
#elif MIRU_TEST_MESH_SHADER
	MeshShader();
#else
	Basic();
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
	g_App = app;
	Basic();
}

#endif