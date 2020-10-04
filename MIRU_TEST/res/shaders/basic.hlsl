#include "msc_common.h"

struct VS_IN
{
    MIRU_LOCATION(0, float4, position, POSITION0);
};

struct VS_OUT
{
    MIRU_LOCATION(0, float4, position, SV_POSITION);
    MIRU_LOCATION(1, float3, texCoords, TEXCOORD1);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
    MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
};
MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);

struct Model
{
    float4x4 modl;
};
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 1, 1, float4, colour);

VS_OUT vs_main(VS_IN IN)
{
    VS_OUT OUT;

	OUT.position = mul(IN.position, mul(model.modl, mul(camera.view, camera.proj)));
	OUT.texCoords = IN.position.xyz;
	return OUT;
}

PS_OUT ps_main(PS_IN IN)
{
    PS_OUT OUT;
	OUT.colour = colour_ImageCIS.Sample(colour_SamplerCIS, IN.texCoords) + (float4(IN.texCoords, 1.0) + float4(0.5, 0.5, 0.5, 0.0));
    return OUT;
    
}