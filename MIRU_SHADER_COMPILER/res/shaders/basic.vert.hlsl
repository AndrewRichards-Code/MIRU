struct VS_IN
{
    float4 position : POSITION;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
};

cbuffer Camera: register(b0)
{
    float4x4 proj;
    float4x4 view;
}

cbuffer Model: register(b1)
{
    float4x4 modl;
}

VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;
    OUT.position = mul(proj * view * modl, IN.position);
	return OUT;
}