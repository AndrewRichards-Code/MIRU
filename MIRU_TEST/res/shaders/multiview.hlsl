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
	float4x4 modl0;
	float4x4 modl1;
};
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 1, 1, float4, colour);

VS_OUT vs_main(VS_IN IN, uint ViewID : SV_ViewID)
{
	VS_OUT OUT;
	float4x4 modl = ViewID == 0 ? model.modl0 : model.modl1;
	
	OUT.position = mul(IN.position, mul(modl, mul(camera.view, camera.proj)));
	OUT.texCoords = IN.position.xyz;
	return OUT;
}

PS_OUT ps_main(PS_IN IN, uint ViewID : SV_ViewID)
{
	PS_OUT OUT;
	//OUT.colour = colour_ImageCIS.Sample(colour_SamplerCIS, IN.texCoords) + (float4(IN.texCoords, 1.0) + float4(0.5, 0.5, 0.5, 0.0));
	float4 c = ViewID == 0 ? float4(1, 0, 0, 1) : float4(0, 1, 0, 1);
	OUT.colour = colour_ImageCIS.Sample(colour_SamplerCIS, IN.texCoords) + c;
	return OUT;
}

struct VS_OUT2
{
	MIRU_LOCATION(0, float4, position, SV_POSITION);
	MIRU_LOCATION(1, float3, texCoords, TEXCOORD1);
};
typedef VS_OUT2 PS_IN2;

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D_ARRAY, 0, 0, float4, colour2);

VS_OUT2 vs_show(uint vertexID : SV_VertexID)
{
	float4 pos[6] =
	{
		float4(-1.0, -1.0, 0.0, 1.0), //0 - Left Lower
		float4(-1.0, +1.0, 0.0, 1.0), //1 - Left Upper
		float4(+0.0, -1.0, 0.0, 1.0), //2 - Centre Lower
		float4(+0.0, +1.0, 0.0, 1.0), //3 - Centre Upper
		float4(+1.0, -1.0, 0.0, 1.0), //4 - Right Lower
		float4(+1.0, +1.0, 0.0, 1.0), //5 - Right Upper
	};
	uint vertexIDToPosIdx[12] =
	{
		0, 2, 3, 3, 1, 0,
		2, 4, 5, 5, 3, 2,
	};
	
	uint vertexIDToPosIdxForTC[12] =
	{
		0, 4, 5, 5, 1, 0,
		0, 4, 5, 5, 1, 0,
	};
	
	VS_OUT2 OUT;
	OUT.position = pos[vertexIDToPosIdx[vertexID]];
	OUT.texCoords.xy = pos[vertexIDToPosIdxForTC[vertexID]].xy / 2.0 + float2(0.5, 0.5);
	#if MIRU_D3D12
	OUT.texCoords.y = 1.0 - OUT.texCoords.y;
	#endif
	OUT.texCoords.z = (vertexID < 6) ? 0.0 : 1.0;
	return OUT;
}

PS_OUT ps_show(PS_IN2 IN)
{
	PS_OUT OUT;
	OUT.colour = colour2_ImageCIS.Sample(colour2_SamplerCIS, IN.texCoords);
	return OUT;
}