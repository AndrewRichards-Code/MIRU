//shader model 6.0

//vertex_input
struct VERTEX_INPUT
{
    float4 positions : SV_POSITION;
};

//fragment_output

//interface_in

//interface_out
struct INTERFACE_OUT
{
    float4 positions : SV_POSITION;
};

//shader_resources
struct CAMERACONSTANTS
{
    float4 proj;
	float4 view;
};
ConstantBuffer<CAMERACONSTANTS> cameraConstants : register(b0);

struct MODELCONSTANTS
{
    float4 modl;
};
ConstantBuffer<MODELCONSTANTS> modelConstants : register(b1);

struct MyStruct
{
    float s_color;
};
RWStructuredBuffer<MyStruct> SSBO : register(u0);
StructuredBuffer<MyStruct> UBO : register(u1);

//functions
INTERFACE_OUT main (VERTEX_INPUT vertex_input)
{
	INTERFACE_OUT interface_out;
	interface_out.positions = cameraConstants.proj * cameraConstants.view * modelConstants.modl * vertex_input.positions;

	return  interface_out;

    SSBO[0].s_color = 1.0f;
    UBO[0].s_color = 1.0f;
}
