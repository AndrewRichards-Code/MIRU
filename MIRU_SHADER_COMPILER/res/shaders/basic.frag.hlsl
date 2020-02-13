struct PS_IN
{
    [[vk::location(0)]] float4 position : SV_POSITION;
};
    
struct PS_OUT
{
    [[vk::location(0)]] float4 colour : SV_TARGET;
};

PS_OUT main(PS_IN IN)
{
    PS_OUT OUT;
    OUT.colour = float4(0.0, 0.0, 1.0, 1.0f);
    return OUT;
}