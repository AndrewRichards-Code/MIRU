//shader model 6.0

//vertex_input
struct VERTEX_INPUT
{
	vec4 positions : SV_POSITION
};

//fragment_output

//interface_in

//interface_out
struct INTERFACE_OUT
{
	vec4 positions : SV_POSITION
};

//shader_resources
struct CAMERACONSTANTS
{
	mat4 proj
	mat4 view
};
ConstantBuffer<CAMERACONSTANTS> cameraConstants : register(b0);

struct MODELCONSTANTS
{
	mat4 modl
};
ConstantBuffer<MODELCONSTANTS> modelConstants : register(b1);


//functions
INTERFACE_OUT main (VERTEX_INPUT vertex_input)
{
	INTERFACE_OUT interface_out;
	interface_out.positions = cameraConstants.proj * cameraConstants.view * modelConstants.modl * vertex_input.positions;

	return  interface_out;
}
