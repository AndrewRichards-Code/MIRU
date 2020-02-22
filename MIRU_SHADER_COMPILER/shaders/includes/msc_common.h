#if defined MIRU_VULKAN
#define MIRU_LOCATION(x, y, z, w) [[vk::location(x)]] y z : w
#define MIRU_CONSTANT_BUFFER(x, y, z, w) [[vk::binding(x, y)]] ConstantBuffer<z> w
#else
#define MIRU_LOCATION(x, y, z, w) y z : w
#define MIRU_CONSTANT_BUFFER(x, y, z, w) ConstantBuffer<z> w : register(b##x, space##y)
#endif

#define MIRU_UNIFORM_BUFFER(x, y, z, w) MIRU_CONSTANT_BUFFER(x, y, z, w)