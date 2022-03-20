#define STB_IMAGE_IMPLEMENTATION
#include "STBI/stb_image.h"

//Forward Declaration from other cpp files.
void Basic();
void Raytracing();
void DynamicRendering();

#define MIRU_TEST_RAYTRACING 0
#define MIRU_TEST_DYNAMIC_RENDERING 0

int main()
{
#if MIRU_TEST_RAYTRACING
	Raytracing();
#elif MIRU_TEST_DYNAMIC_RENDERING
	DynamicRendering();
#else
	Basic();
#endif

}