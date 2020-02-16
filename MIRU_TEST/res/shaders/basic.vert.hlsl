struct VS_IN
{
#ifdef MIRU_VULKAN
    [[vk::location(0)]] float4 position : POSITION;
#else
    float4 position : POSITION;
#endif
};

struct VS_OUT
{
#ifdef MIRU_VULKAN
    [[vk::location(0)]] float4 position     : SV_POSITION;
    [[vk::location(1)]] float2 texCoords    : TEXCOORD0;
#else
    float4 position     : SV_POSITION;
    float2 texCoords    : TEXCOORD0;
#endif
};

/*struct Camera
{
    float4x4 proj;
    float4x4 view;
};
[[vk::binding(0, 0)]] ConstantBuffer<Camera> camera;

struct Model
{
    float4x4 modl;
};
[[vk::binding(1, 0)]] ConstantBuffer<Model> model;*/


VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;
    //OUT.position = mul(camera.proj * camera.view * model.modl, IN.position);
    OUT.position = IN.position;
    OUT.texCoords = IN.position.xy + float2(0.5, 0.5);
	return OUT;
}