#include "msc_common.h"

struct Vertex
{
	float4 position : SV_POSITION;
	float3 texCoords : TEXCOORD1;
};
typedef Vertex PS_IN;

struct Meshlet
{
	uint VertexCount;
	uint VertexOffset;
	uint PrimitiveCount;
	uint PrimitiveOffset;
};

struct Primitive
{
	bool cull : SV_CullPrimitive;
};

MIRU_STRUCTURED_BUFFER(2, 0, Meshlet, meshlets);
MIRU_STRUCTURED_BUFFER(2, 1, Vertex, inVertices);
MIRU_STRUCTURED_BUFFER(2, 2, uint, inVertexIdxs);
MIRU_STRUCTURED_BUFFER(2, 3, uint3, inPrimitveIdxs);

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

void as_main()
{
}

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void ms_main(
	in uint g : SV_GroupID, //Which meshlet?
	in uint gt : SV_GroupThreadID, //Which vertex/primitive within the meshlet?
	out vertices Vertex outVerts[128],
	out indices uint3 outIndices[128]/*, 
	out primitives Primitive outPrims[128]*/
)
{
	Meshlet meshlet = meshlets[g];
	SetMeshOutputCounts(meshlet.VertexCount, meshlet.PrimitiveCount);
	
	//Write the vertices
	if (gt < meshlet.VertexCount)
	{
		uint finalVertID = inVertexIdxs[gt + meshlet.VertexOffset];
		Vertex vertex = inVertices[finalVertID];
		
		outVerts[gt].position = mul(vertex.position, mul(model.modl, mul(camera.view, camera.proj)));
		outVerts[gt].texCoords = vertex.position.xyz;
	}

	//Write the indices
	if (gt < meshlet.PrimitiveCount)
	{
		uint3 _indices = inPrimitveIdxs[gt + meshlet.PrimitiveOffset];
		outIndices[gt] = _indices;
	}
	
	/*
	//Sync all thread for primitive culling
	GroupMemoryBarrierWithGroupSync();
	
	//Write primitive attributes.
	if (gt < meshlet.PrimitiveCount && gt%3 == 0)
	{
		uint3 _indices = outIndices[gt];
		float4 vertex0 = outVerts[_indices.x].position;
		float4 vertex1 = outVerts[_indices.y].position;
		float4 vertex2 = outVerts[_indices.z].position;
		float3 normal = normalize(cross(vertex1.xyz - vertex0.xyz, vertex2.xyz - vertex0.xyz));
		bool cull = dot(normal, float3(0.0, 0.0, 1.0)) > 0.0;
		outPrims[gt].cull = cull;
	}*/
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	OUT.colour = colour_ImageCIS.Sample(colour_SamplerCIS, IN.texCoords) + (float4(IN.texCoords, 1.0) + float4(0.5, 0.5, 0.5, 0.0));
	return OUT;
	
}