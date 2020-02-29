//Vertex Inputs
#if defined MIRU_VULKAN
#define MIRU_LOCATION(x, y, z, w) [[vk::location(x)]] y z : w
#else
#define MIRU_LOCATION(x, y, z, w) y z : w
#endif

//Constant Buffers / Uniform Buffers
#if defined MIRU_VULKAN
#define MIRU_CONSTANT_BUFFER(x, y, z, w) [[vk::binding(x, y)]] ConstantBuffer<z> w
#else
#define MIRU_CONSTANT_BUFFER(x, y, z, w) ConstantBuffer<z> w : register(b##x, space##y)
#endif
#define MIRU_UNIFORM_BUFFER(x, y, z, w) MIRU_CONSTANT_BUFFER(x, y, z, w)

//Images and Samplers
#if defined MIRU_VULKAN
#define MIRU_IMAGE_1D(x, y, z, w) [[vk::binding(x, y)]] Texture1D<z> w 
#define MIRU_IMAGE_2D(x, y, z, w) [[vk::binding(x, y)]] Texture2D<z> w 
#define MIRU_IMAGE_3D(x, y, z, w) [[vk::binding(x, y)]] Texture3D<z> w 
#define MIRU_IMAGE_CUBE(x, y, z, w) [[vk::binding(x, y)]] TextureCube<z> w 
#define MIRU_IMAGE_1D_ARRAY(x, y, z, w) [[vk::binding(x, y)]] Texture1DArray<z> w 
#define MIRU_IMAGE_2D_ARRAY(x, y, z, w) [[vk::binding(x, y)]] Texture2DArray<z> w 
#define MIRU_IMAGE_CUBE_ARRAY(x, y, z, w) [[vk::binding(x, y)]] TextureCubeArray<z> w 
#define MIRU_RW_IMAGE_1D(x, y, z, w) RW[[vk::binding(x, y)]] Texture1D<z> w 
#define MIRU_RW_IMAGE_2D(x, y, z, w) RW[[vk::binding(x, y)]] Texture2D<z> w 
#define MIRU_RW_IMAGE_3D(x, y, z, w) RW[[vk::binding(x, y)]] Texture3D<z> w 
#define MIRU_RW_IMAGE_1D_ARRAY(x, y, z, w) RW[[vk::binding(x, y)]] Texture1DArray<z> w 
#define MIRU_RW_IMAGE_2D_ARRAY(x, y, z, w) RW[[vk::binding(x, y)]] Texture2DArray<z> w 
#define MIRU_SAMPLER(x, y, z) [[vk::binding(x, y)]] SamplerState z
//The name component of the image is defined as 'name_image', and the name component of the sampler is defined as 'name_sampler'.
#define MIRU_COMBINED_IMAGE_SAMPLER(type, x, y, z, w) type(x, y, z, w##_image); MIRU_SAMPLER(x, y, w##_sampler)
#else
#define MIRU_IMAGE_1D(x, y, z, w) Texture1D<z> w : register(t##x, space##y)
#define MIRU_IMAGE_2D(x, y, z, w) Texture2D<z> w : register(t##x, space##y)
#define MIRU_IMAGE_3D(x, y, z, w) Texture3D<z> w : register(t##x, space##y)
#define MIRU_IMAGE_CUBE(x, y, z, w) TextureCube<z> w : register(t##x, space##y)
#define MIRU_IMAGE_1D_ARRAY(x, y, z, w) Texture1DArray<z> w : register(t##x, space##y)
#define MIRU_IMAGE_2D_ARRAY(x, y, z, w) Texture2DArray<z> w : register(t##x, space##y)
#define MIRU_IMAGE_CUBE_ARRAY(x, y, z, w) TextureCubeArray<z> w : register(t##x, space##y)
#define MIRU_RW_IMAGE_1D(x, y, z, w) RWTexture1D<z> w : register(t##x, space##y)
#define MIRU_RW_IMAGE_2D(x, y, z, w) RWTexture2D<z> w : register(t##x, space##y)
#define MIRU_RW_IMAGE_3D(x, y, z, w) RWTexture3D<z> w : register(t##x, space##y)
#define MIRU_RW_IMAGE_1D_ARRAY(x, y, z, w) RWTexture1DArray<z> w : register(t##x, space##y)
#define MIRU_RW_IMAGE_2D_ARRAY(x, y, z, w) RWTexture2DArray<z> w : register(t##x, space##y)
#define MIRU_SAMPLER(x, y, z) SamplerState z : register(s##x, space##y)
//The name component of the image is defined as 'name_image', and the name component of the sampler is defined as 'name_sampler'.
#define MIRU_COMBINED_IMAGE_SAMPLER(type, x, y, z, w) type(x, y, z, w##_image); MIRU_SAMPLER(x, y, w##_sampler)
#endif