struct PS_IN
{
#ifdef MIRU_VULKAN
    [[vk::location(0)]] float4 position     : SV_POSITION;
    [[vk::location(1)]] float2 texCoords    : TEXCOORD0;
#else
    float4 position     : SV_POSITION;
    float2 texCoords    : TEXCOORD0;
#endif
};
    
struct PS_OUT
{
#ifdef MIRU_VULKAN
    [[vk::location(0)]] float4 colour       : SV_TARGET;
#else
    float4 colour       : SV_TARGET;
#endif
};

PS_OUT main(PS_IN IN)
{
    PS_OUT OUT;
    OUT.colour = float4(IN.texCoords, 0.0, 1.0);
    return OUT;
}