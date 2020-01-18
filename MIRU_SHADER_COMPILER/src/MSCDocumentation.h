#pragma once
namespace miru
{
namespace shader_compiler
{
	const char* help_doucumentation = 
R"(MIRU_SHADER_COMPILER: Help Documentation:

The MIRU_SHADER_COMPILER takes .<shader_type>.hlsl file and 
builds .cso or .spv shader binaries for Direct3D 12 and Vulkan.

-h, -H, -help, -HELP        : For this help documentation. Optional.
-d - D - debug -DEBUG       : For debugging shader compilation, sets the -h flag. Optional.
-f:, -F:[filepath]          : Filepath to a .hlsl file to be compiled. This argument must be set.
-o:, -O:[directory]         : Directory for the output binaries. Default is the supplied filepath.
-e: -E:                     : Define an entry point. Default is "main".
-dxc -DXC                   : Specify location of the dxc.exe. Optional.
-glslang -GLSLANG           : Specify location of the glslangValidator.exe. Optional.
-cso, -CSO                  : Generates binaries(.cso) for Direct3D 12. This argument or -spv must be set.
-spv, -SPV                  : Generates binaries(.spv) for Vulkan. This argument or -cso must be set.
-vi, -VI                    : Generates Vertex Input Info. Optional.
-sr, -SR                    : Generates Shader Resources Info. Optional.
-fo, -FO, -po, -PO          : Generates Fragment/Pixel Output Info. Optional.

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
.conf  to provide a config file that replaces the default configuration
)";
}
}