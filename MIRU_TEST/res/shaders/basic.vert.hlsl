#include "msc_common.h"

struct VS_IN
{
    MIRU_LOCATION(0, float4, position, POSITION0);
    MIRU_LOCATION(1, float2, texCoords, TEXCOORD1);
};

struct VS_OUT
{
    MIRU_LOCATION(0, float4, position, SV_POSITION);
    MIRU_LOCATION(1, float2, texCoords, TEXCOORD1);
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

VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;

	OUT.position = mul(IN.position, mul(model.modl, mul(camera.view, camera.proj)));
	OUT.texCoords = IN.texCoords;
	return OUT;
}