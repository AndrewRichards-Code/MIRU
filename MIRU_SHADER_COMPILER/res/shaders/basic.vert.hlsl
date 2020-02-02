struct VS_IN
{
    [[vk::location(0)]] float4 position : POSITION;
};

struct VS_OUT
{
    [[vk::location(0)]] float4 position : SV_POSITION;
};

/*struct Camera
{
    float4x4 proj;
    float4x4 view;
};
[[vk::binding(0, 0)]] ConstantBuffer<Camera> camera;

struct Model
{
    float4x4 modl;
};
[[vk::binding(1, 0)]] ConstantBuffer<Model> model;*/


VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;
    //OUT.position = mul(camera.proj * camera.view * model.modl, IN.position);
    OUT.position = IN.position;
	return OUT;
}