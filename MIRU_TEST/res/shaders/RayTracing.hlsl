#include "msc_common.h"

struct Camera
{
	float4x4 viewMatrix;
	float3 position;
	float aspectRatio;
	float4 direction;
};
struct SceneConstants
{
	Camera camera;
	float TMin;
	float TMax;
};
MIRU_UNIFORM_BUFFER(0, 0, SceneConstants, scene);
MIRU_RW_IMAGE_2D(0, 1, float4, output);
MIRU_RAYTRACING_ACCELERATION_STRUCTURE(0, 2, accelStruct);

MIRU_SAMPLER(1, 0, imageSampler);
MIRU_STRUCTURED_BUFFER(1, 1, float4, _vertices);
MIRU_STRUCTURED_BUFFER(1, 2, uint, _indices);
MIRU_IMAGE_2D(1, 3, float4, imageTexture[]);

struct Payload
{
	float4 colour;
};

float4 CubemapFaceColour(uint faceIndex)
{
	float4 result = float4(0.0, 0.0, 0.0, 1.0);

	switch (faceIndex)
	{
	case 0: result = float4(1.0, 0.0, 0.0, 1.0); break;
	case 1: result = float4(0.0, 1.0, 1.0, 1.0); break;
	case 2: result = float4(0.0, 1.0, 0.0, 1.0); break;
	case 3: result = float4(1.0, 0.0, 1.0, 1.0); break;
	case 4: result = float4(0.0, 0.0, 1.0, 1.0); break;
	case 5: result = float4(1.0, 1.0, 0.0, 1.0); break;
	}

	return result;
}

//https://gamedev.net/forums/topic/687535-implementing-a-cube-map-lookup-function/5337472/#post-5337472
float2 UVWToFaceUV(const float3 uvw, out uint faceIndex)
{
	float3 uvwAbs = abs(uvw);
	float scaledAxis;
	float2 uv;
	if (uvwAbs.z >= uvwAbs.x && uvwAbs.z >= uvwAbs.y)
	{
		faceIndex = uvw.z < 0.0 ? 5 : 4;
		scaledAxis = 0.5 / uvwAbs.z;
		uv = float2(uvw.z < 0.0 ? -uvw.x : uvw.x, -uvw.y);
	}
	else if (uvwAbs.y >= uvwAbs.x)
	{
		faceIndex = uvw.y < 0.0 ? 3 : 2;
		scaledAxis = 0.5 / uvwAbs.y;
		uv = float2(uvw.x, uvw.y < 0.0 ? -uvw.z : uvw.z);
	}
	else
	{
		faceIndex = uvw.x < 0.0 ? 1 : 0;
		scaledAxis = 0.5 / uvwAbs.x;
		uv = float2(uvw.x < 0.0 ? uvw.z : -uvw.z, -uvw.y);
	}
	return (uv * scaledAxis) + float2(0.5, 0.5);
}

float3 GetCameraToPixelRayDirection(Camera camera)
{
	uint2 pixelID = DispatchRaysIndex().xy;
	uint2 dimension = DispatchRaysDimensions().xy;
	float2 normalisedCentrePixelID = 2.0 * (float2(pixelID)/float2(dimension)) - float2(1.0, 1.0);
	float4 scaledViewPixelID_F = normalize(float4(normalisedCentrePixelID.x * camera.aspectRatio, normalisedCentrePixelID.y, -1.0, 0.0)); 
	float3 viewDirection = normalize(mul(camera.viewMatrix, scaledViewPixelID_F)).xyz;
	return viewDirection;
}

[shader("raygeneration")]
void ray_generation_main()
{
	RayDesc ray;
	ray.Origin = scene.camera.position;
	ray.TMin = scene.TMin;
	ray.Direction = GetCameraToPixelRayDirection(scene.camera);
	ray.TMax = scene.TMax;
	
	Payload payload = { float4(0,0,0,0) };
	
	TraceRay(accelStruct, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
	
	output[DispatchRaysIndex().xy] = payload.colour;

}

[shader("anyhit")]
void any_hit_main(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
	payload.colour = float4(1.0, 0.0, 0.0, 1.0);
}

[shader("closesthit")]
void closest_hit_main(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
	//payload.colour = float4(attr.barycentrics, 0.0, 1.0);
	float3 barycentrics = float3(1.0f - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
	
	uint indexID = PrimitiveIndex() * 3;
	uint index0 = _indices[indexID + 0];
	uint index1 = _indices[indexID + 1];
	uint index2 = _indices[indexID + 2];
	
	float4 vertex0 = _vertices[index0];
	float4 vertex1 = _vertices[index1];
	float4 vertex2 = _vertices[index2];
	
	float3 uvw0 = vertex0.xyz;
	float3 uvw2 = vertex2.xyz;
	float3 uvw1 = vertex1.xyz;
	float3 uvw = barycentrics.x * uvw0 + barycentrics.y * uvw1 + barycentrics.z * uvw2;
	
	uint faceIndex;
	float2 faceUV = UVWToFaceUV(-uvw, faceIndex);
	float4 colour = CubemapFaceColour(faceIndex);
	
	float4 imageColour = imageTexture[NonUniformResourceIndex(0)].SampleLevel(imageSampler, faceUV, 0);
	payload.colour = lerp(imageColour, colour, 0.1);

}

[shader("miss")]
void miss_main(inout Payload payload)
{
	payload.colour = float4(0.0, 0.0, 1.0, 1.0);

}