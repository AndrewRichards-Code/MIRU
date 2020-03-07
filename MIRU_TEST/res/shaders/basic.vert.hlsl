#include "msc_common.h"

struct VS_IN
{
    MIRU_LOCATION(0, float4, position, POSITION);
    MIRU_LOCATION(1, float2, texCoords, TEXCOORD);
};

struct VS_OUT
{
    MIRU_LOCATION(0, float4, position, SV_POSITION);
    MIRU_LOCATION(1, float2, texCoords, TEXCOORD);
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
MIRU_UNIFORM_BUFFER(0, 1, Model, model);

VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;

    OUT.position = mul(camera.proj * camera.view * model.modl, IN.position);
	//OUT.position = IN.position;
	//OUT.texCoords = IN.position.xy + float2(0.5, 0.5);
	OUT.texCoords = IN.texCoords;
	return OUT;
}