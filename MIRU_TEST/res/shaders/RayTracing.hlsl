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
//MIRU_SHADER_RECORD(MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 0, float4, LRS_texture));

struct Payload
{
	float4 colour;
};

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
	payload.colour = float4(attr.barycentrics, 0.0, 1.0);
	//payload.colour = LRS_texture_ImageCIS.SampleLevel(LRS_texture_SamplerCIS, float2(0.5, 0.5), 0);
}

[shader("miss")]
void miss_main(inout Payload payload)
{
	payload.colour = float4(0.0, 0.0, 1.0, 1.0);

}