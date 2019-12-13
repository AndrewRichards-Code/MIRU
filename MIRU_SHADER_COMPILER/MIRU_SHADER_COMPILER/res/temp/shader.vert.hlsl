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
    matrix proj;
	matrix view;
};
ConstantBuffer<CAMERACONSTANTS> cameraConstants : register(b0);

struct MODELCONSTANTS
{
    matrix modl;
};
ConstantBuffer<MODELCONSTANTS> modelConstants : register(b1);

//functions
INTERFACE_OUT main(VERTEX_INPUT vertex_input)
{
    INTERFACE_OUT interface_out;
    interface_out.positions = mul((cameraConstants.proj * cameraConstants.view * modelConstants.modl), vertex_input.positions);
    
	return interface_out;
}