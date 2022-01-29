#define STB_IMAGE_IMPLEMENTATION
#include "../dep/STBI/stb_image.h"

//Forward Declaration from other cpp files.
void Basic();
void Raytracing();

#define MIRU_TEST_RT 0

int main()
{
#if MIRU_TEST_RT
	Raytracing();
#else
	Basic();
#endif

}