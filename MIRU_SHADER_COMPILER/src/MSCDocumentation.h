#pragma once
namespace miru
{
namespace shader_compiler
{
	const char* help_doucumentation = 
R"(MIRU_SHADER_COMPILER: Help Documentation:
The MIRU_SHADER_COMPILER takes .<shader_type>.hlsl file and builds .cso or .spv shader binaries for Direct3D 12 and Vulkan.

-h, -H, -help, -HELP                : For this help documentation. Optional.
-pause -PAUSE                       : Pauses the program at the end of shader compilation, sets the -h flag. Optional.
-nologo, -NOLOGO                    : Disables copyright message. Optional.
-nooutput, -NOOUTPUT                : Disables output messages. Optional.
-f:, -F:[filepath]                  : Filepath to a .hlsl file to be compiled. This argument must be set.
-o:, -O:[directory]                 : Directory for the output binaries. Default is the filepath directory.
-i:, -I:[directory]                 : Include directory for compilation. Optional.
-e:, -E:                            : Define an entry point. Default is: main.
-t:, -T:                            : Define the shader model for Direct3D 12. Default is "6_0".
-d, -D                              : Define a marco. MIRU_D3D12 for -cso and MIRU_VULKAN for -spv are already pre-defined.
-dxc:, -DXC:[directory]             : Specify location of the dxc.exe. Optional.
-glslang:, -GLSLANG:[directory]     : Specify location of the glslangValidator.exe. Optional.
-cso, -CSO                          : Generates binaries(.cso) for Direct3D 12. This argument or -spv must be set.
-spv, -SPV                          : Generates binaries(.spv) for Vulkan. This argument or -cso must be set.
-args:, -ARGS:["..."]               : Provide additional arguments directly to dxc or glslangValidator. Enclose all arguments in double quotation marks e.g. -args:"-Zi -Od".

Supported Shader Types <shader_type>:
vert   for a vertex shader
tesc   for a tessellation control shader
tese   for a tessellation evaluation shader
geom   for a geometry shader
frag   for a fragment shader
comp   for a compute shader
mesh   for a mesh shader
task   for a task shader
rgen   for a ray generation shader
rint   for a ray intersection shader
rahit  for a ray any hit shader
rchit  for a ray closest hit shader
rmiss  for a ray miss shader
rcall  for a ray callable shader
conf   to provide a config file that replaces the default configuration
)";
}
}