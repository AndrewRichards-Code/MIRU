#version 450 core

//vertex_input

//fragment_output
layout(location = 0) out vec4 colour;

//interface_in

//interface_out

//shader_resources
layout(std140, binding = 0) uniform fragmentConstants
{
	vec4 u_colour;
};
layout(binding = 1) uniform texture2D sampledTexture;
layout(binding = 2) uniform sampler sampler1;

layout(std430, binding = 3) buffer ssbo
{
	float s_colour;
};

//functions
void main()
{
	colour = u_colour;
	colour = texture(sampler2D(sampledTexture, sampler1), u_colour.xy);
}
