//Vertex Inputs / Fragment or Pixel Outputs
#if defined MIRU_VULKAN
#define MIRU_LOCATION(loc_num, type, name, semantic) [[vk::location(loc_num)]] type name : semantic
#define MIRU_LOCATION_INDEX(loc_num, idx_num, type, name, semantic) [[vk::location(loc_num)]][[vk::index(idx_num)]] type name : semantic
#else
#define MIRU_LOCATION(loc_num, type, name, semantic) type name : semantic
#define MIRU_LOCATION_INDEX(loc_num, idx_num, type, name, semantic) type name : semantic
#endif

//Constant Buffers / Uniform Buffers
#if defined MIRU_VULKAN
#define MIRU_CONSTANT_BUFFER(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] ConstantBuffer<type> name
#else
#define MIRU_CONSTANT_BUFFER(set_num, bind_num, type, name) ConstantBuffer<type> name : register(b##bind_num, space##set_num)
#endif
#define MIRU_UNIFORM_BUFFER(set_num, bind_num, type, name) MIRU_CONSTANT_BUFFER(set_num, bind_num, type, name)

//RWStructured Buffers / Storage Buffers
#if defined MIRU_VULKAN
#define MIRU_RW_STRUCTURED_BUFFER(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] RWStructuredBuffer<type> name
#else
#define MIRU_RW_STRUCTURED_BUFFER(set_num, bind_num, type, name) RWStructuredBuffer<type> name : register(u##bind_num, space##set_num)
#endif
#define MIRU_STORAGE_BUFFER(set_num, bind_num, type, name) MIRU_RW_STRUCTURED_BUFFER(set_num, bind_num, type, name)

//Structured Buffers 
#if defined MIRU_VULKAN
#define MIRU_STRUCTURED_BUFFER(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] StructuredBuffer<type> name
#else
#define MIRU_STRUCTURED_BUFFER(set_num, bind_num, type, name) StructuredBuffer<type> name : register(t##bind_num, space##set_num)
#endif

//Images and Samplers
#if defined MIRU_VULKAN
#define MIRU_IMAGE_1D(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture1D<type> name 
#define MIRU_IMAGE_2D(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture2D<type> name 
#define MIRU_IMAGE_3D(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture3D<type> name 
#define MIRU_IMAGE_CUBE(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] TextureCube<type> name 
#define MIRU_IMAGE_1D_ARRAY(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture1DArray<type> name 
#define MIRU_IMAGE_2D_ARRAY(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture2DArray<type> name 
#define MIRU_IMAGE_CUBE_ARRAY(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] TextureCubeArray<type> name 
#define MIRU_IMAGE_2DMS(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture2DMS<type> name 
#define MIRU_IMAGE_2DMS_ARRAY(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] Texture2DMSArray<type> name 
#define MIRU_RW_IMAGE_1D(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture1D<type> name 
#define MIRU_RW_IMAGE_2D(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture2D<type> name 
#define MIRU_RW_IMAGE_3D(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture3D<type> name 
#define MIRU_RW_IMAGE_1D_ARRAY(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture1DArray<type> name 
#define MIRU_RW_IMAGE_2D_ARRAY(set_num, bind_num, type, name) [[vk::binding(bind_num, set_num)]] RWTexture2DArray<type> name 
#define MIRU_SAMPLER(set_num, bind_num, name) [[vk::binding(bind_num, set_num)]] SamplerState name
#else
#define MIRU_IMAGE_1D(set_num, bind_num, type, name) Texture1D<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_2D(set_num, bind_num, type, name) Texture2D<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_3D(set_num, bind_num, type, name) Texture3D<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_CUBE(set_num, bind_num, type, name) TextureCube<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_1D_ARRAY(set_num, bind_num, type, name) Texture1DArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_2D_ARRAY(set_num, bind_num, type, name) Texture2DArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_CUBE_ARRAY(set_num, bind_num, type, name) TextureCubeArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_2DMS(set_num, bind_num, type, name) Texture2DMS<type> name : register(t##bind_num, space##set_num)
#define MIRU_IMAGE_2DMS_ARRAY(set_num, bind_num, type, name) Texture2DMSArray<type> name : register(t##bind_num, space##set_num)
#define MIRU_RW_IMAGE_1D(set_num, bind_num, type, name) RWTexture1D<type> name : register(u##bind_num, space##set_num)
#define MIRU_RW_IMAGE_2D(set_num, bind_num, type, name) RWTexture2D<type> name : register(u##bind_num, space##set_num)
#define MIRU_RW_IMAGE_3D(set_num, bind_num, type, name) RWTexture3D<type> name : register(u##bind_num, space##set_num)
#define MIRU_RW_IMAGE_1D_ARRAY(set_num, bind_num, type, name) RWTexture1DArray<type> name : register(u##bind_num, space##set_num)
#define MIRU_RW_IMAGE_2D_ARRAY(set_num, bind_num, type, name) RWTexture2DArray<type> name : register(u##bind_num, space##set_num)
#define MIRU_SAMPLER(set_num, bind_num, name) SamplerState name : register(s##bind_num, space##set_num)
#endif

//The name component of the image is defined as 'name_ImageCIS', and the name component of the sampler is defined as 'name_SamplerCIS'.
#define MIRU_COMBINED_IMAGE_SAMPLER(image_type, set_num, bind_num, type, name) image_type(set_num, bind_num, type, name##_ImageCIS); MIRU_SAMPLER(set_num, bind_num, name##_SamplerCIS)
#define MIRU_COMBINED_IMAGE_SAMPLER_ARRAY(image_type, set_num, bind_num, type, name, count) image_type(set_num, bind_num, type, name##_ImageCIS[count]); MIRU_SAMPLER(set_num, bind_num, name##_SamplerCIS[count])

//Subpass Input Attachments
#if defined MIRU_VULKAN && defined MIRU_FRAGMENT_SHADER
#define MIRU_SUBPASS_INPUT(set_num, bind_num, idx_num, type, name) [[vk::binding(bind_num, set_num)]][[vk::input_attachment_index(idx_num)]] SubpassInput<type> name
#define MIRU_SUBPASS_INPUT_MS(set_num, bind_num, idx_num, type, name) [[vk::binding(bind_num, set_num)]][[vk::input_attachment_index(idx_num)]] SubpassInputMS<type> name
#define MIRU_SUBPASS_LOAD(name, sv_pos) name.SubpassLoad()
#define MIRU_SUBPASS_LOAD_MS(name, sv_pos, sampleIdx) name.SubpassLoad(sampleIdx)
#else
#define MIRU_SUBPASS_INPUT(set_num, bind_num, idx_num, type, name) MIRU_IMAGE_2D(set_num, bind_num, type, name)
#define MIRU_SUBPASS_INPUT_MS(set_num, bind_num, idx_num, type, name) MIRU_IMAGE_2DMS(set_num, bind_num, type, name)
#define MIRU_SUBPASS_LOAD(name, sv_pos) name.Load(int3(sv_pos.xy, 0))
#define MIRU_SUBPASS_LOAD_MS(name, sv_pos, sampleIdx) name.Load(int2(sv_pos.xy), sampleIdx)
#endif

//Compute Shaders

//Number of Threads per Group
#define MIRU_COMPUTE_LAYOUT(x, y, z) [numthreads(x, y, z)]
//The GroupID within the Dispatch.
#define MIRU_GROUP_ID SV_GroupID
//The ThreadID within the Group.
#define MIRU_GROUP_THREAD_ID SV_GroupThreadID
//The ThreadID within the whole Dispatch.
#define MIRU_DISPATCH_THREAD_ID SV_DispatchThreadID
//The "flattened" index of the Thread within the Group
#define MIRU_GROUP_INDEX SV_GroupIndex

//Ray Tracing Shaders
#if defined MIRU_VULKAN
#define MIRU_RAYTRACING_ACCELERATION_STRUCTURE(set_num, bind_num, name) [[vk::binding(bind_num, set_num)]] RaytracingAccelerationStructure name
#define MIRU_LOCAL_ROOT_SIGNATURE_PARAMETER(parameter_decl) [[vk::shader_record_ext]] parameter_decl
#else
#define MIRU_RAYTRACING_ACCELERATION_STRUCTURE(set_num, bind_num, name) RaytracingAccelerationStructure name : register(t##bind_num, space##set_num)
#define MIRU_LOCAL_ROOT_SIGNATURE_PARAMETER(parameter_decl) parameter_decl
#endif
#define MIRU_SHADER_RECORD(parameter_decl) MIRU_LOCAL_ROOT_SIGNATURE_PARAMETER(parameter_decl)

//Mesh Shaders

//Define the Output Topology for the rasteriser. For use only "line" or "triangle".
#define MIRU_OUTPUT_TOPOLOGY(type) [outputtopology(type)]

//NDC for D3D12 and Vulkan
//https://github.com/gpuweb/gpuweb/issues/416
