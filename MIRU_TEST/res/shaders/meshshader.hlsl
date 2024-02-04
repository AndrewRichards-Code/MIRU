#include "msc_common.h"

struct VertexOut
{
	float4 position : SV_POSITION;
	float3 texCoords : TEXCOORD1;
};
typedef VertexOut PS_IN;

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

struct Vertex
{
	float4 position;
};
MIRU_STRUCTURED_BUFFER(1, 2, Vertex, _vertices);

struct Meshlet
{
	uint vertices[64];
	uint indices[126]; // up to 126 triangles
	uint indexCount;
	uint vertexCount;
};
MIRU_STRUCTURED_BUFFER(1, 3, Meshlet, meshlets);

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void ms_main(
	in uint g : SV_GroupID, //Which meshlet?
	out vertices VertexOut outVerts[64],
	out indices uint3 outIndices[42]
)
{
	Meshlet meshlet = meshlets[g];
	uint primitiveCount = meshlet.indexCount / 3;

	SetMeshOutputCounts(meshlet.vertexCount, primitiveCount);
	
	//Write the vertices
	for (uint i = 0; i < meshlet.vertexCount; i++)
	{
		Vertex vertex = _vertices[meshlet.vertices[i]];
		
		outVerts[i].position = mul(vertex.position, mul(model.modl, mul(camera.view, camera.proj)));
		outVerts[i].texCoords = vertex.position.xyz;
	}

	//Write the indices
	for (uint j = 0; j < primitiveCount; j++)
	{
		uint a = meshlet.indices[j * 3 + 0];
		uint b = meshlet.indices[j * 3 + 1];
		uint c = meshlet.indices[j * 3 + 2];

		outIndices[j] = uint3(a, b, c);
	}
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	OUT.colour = colour_ImageCIS.Sample(colour_SamplerCIS, IN.texCoords) + (float4(IN.texCoords, 1.0) + float4(0.5, 0.5, 0.5, 0.0));
	return OUT;
	
}