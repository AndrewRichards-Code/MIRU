#include "msc_common.h"

struct VS_OUT
{
    MIRU_LOCATION(0, float4, position, SV_POSITION);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
    MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};
MIRU_SUBPASS_INPUT(0, 0, 0, float4, input);

VS_OUT vs_main(uint VertexIndex : SV_VertexID)
{
	VS_OUT OUT;
	OUT.position = float4(float2((VertexIndex << 1) & 2, VertexIndex & 2) * 2.0f - 1.0f, 0.0f, 1.0f);
	return OUT;
}

PS_OUT ps_main(PS_IN IN )
{
	PS_OUT OUT;
	float4 colour = MIRU_SUBPASS_LOAD(input, IN.position);
	float values = (colour.r + colour.g + colour.b) / 3.0;
	OUT.colour = pow(float4(values, values, values, colour.a), 2.2);
	return OUT;
}

