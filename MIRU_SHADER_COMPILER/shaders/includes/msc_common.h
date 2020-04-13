//Vertex Inputs / Fragment or Pixel Outputs
#if defined MIRU_VULKAN
#define MIRU_LOCATION(loc_num, type, name, semantic) [[vk::location(loc_num)]] type name : semantic
#define MIRU_LOCATION_INDEX(loc_num, idx_num, type, name, semantic) [[vk::location(loc_num), vk::index(idx_num)]] type name : semantic
#else
#define MIRU_LOCATION(loc_num, type, name, semantic) type name : semantic
#define MIRU_LOCATION_INDEX(loc_num, idx_num, type, name, semantic) type name : semantic
#endif

//Constant Buffers / Uniform Buffers
#if defined MIRU_VULKAN
#define MIRU_CONSTANT_BUFFER(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] ConstantBuffer<type> name
#else
#define MIRU_CONSTANT_BUFFER(bind_num, set_num, type, name) ConstantBuffer<type> name : register(b##bind_num, space##set_num)
#endif
#define MIRU_UNIFORM_BUFFER(bind_num, set_num, type, name) MIRU_CONSTANT_BUFFER(bind_num, set_num, type, name)

//Images and Samplers
#if defined MIRU_VULKAN
#define MIRU_IMAGE_1D(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] Texture1D<type> name 
#define MIRU_IMAGE_2D(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] Texture2D<type> name 
#define MIRU_IMAGE_3D(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] Texture3D<type> name 
#define MIRU_IMAGE_CUBE(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] TextureCube<type> name 
#define MIRU_IMAGE_1D_ARRAY(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] Texture1DArray<type> name 
#define MIRU_IMAGE_2D_ARRAY(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] Texture2DArray<type> name 
#define MIRU_IMAGE_CUBE_ARRAY(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] TextureCubeArray<type> name 
#define MIRU_RW_IMAGE_1D(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture1D<type> name 
#define MIRU_RW_IMAGE_2D(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture2D<type> name 
#define MIRU_RW_IMAGE_3D(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture3D<type> name 
#define MIRU_RW_IMAGE_1D_ARRAY(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture1DArray<type> name 
#define MIRU_RW_IMAGE_2D_ARRAY(bind_num, set_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture2DArray<type> name 
#define MIRU_SAMPLER(bind_num, set_num, name) [[vk::binding(bind_num, set_num)]] SamplerState name
#else
#define MIRU_IMAGE_1D(bind_num, set_num, type, name) Texture1D<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_2D(bind_num, set_num, type, name) Texture2D<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_3D(bind_num, set_num, type, name) Texture3D<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_CUBE(bind_num, set_num, type, name) TextureCube<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_1D_ARRAY(bind_num, set_num, type, name) Texture1DArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_2D_ARRAY(bind_num, set_num, type, name) Texture2DArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_CUBE_ARRAY(bind_num, set_num, type, name) TextureCubeArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_RW_IMAGE_1D(bind_num, set_num, type, name) RWTexture1D<type> name : register(t##bind_num, space##set_num)
#define MIRU_RW_IMAGE_2D(bind_num, set_num, type, name) RWTexture2D<type> name : register(t##bind_num, space##set_num)
#define MIRU_RW_IMAGE_3D(bind_num, set_num, type, name) RWTexture3D<type> name : register(t##bind_num, space##set_num)
#define MIRU_RW_IMAGE_1D_ARRAY(bind_num, set_num, type, name) RWTexture1DArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_RW_IMAGE_2D_ARRAY(bind_num, set_num, type, name) RWTexture2DArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_SAMPLER(bind_num, set_num, name) SamplerState name : register(s##bind_num, space##set_num)
#endif
//The name component of the image is defined as 'name_image_cis', and the name component of the sampler is defined as 'name_sampler_cis'.
#define MIRU_COMBINED_IMAGE_SAMPLER(image_type, bind_num, set_num, type, name) image_type(bind_num, set_num, type, name##_image_cis); MIRU_SAMPLER(bind_num, set_num, name##_sampler_cis)
#define MIRU_COMBINED_IMAGE_SAMPLER_ARRAY(image_type, bind_num, set_num, type, name, count) image_type(bind_num, set_num, type, name##_image_cis[count]); MIRU_SAMPLER(bind_num, set_num, name##_sampler_cis[count])