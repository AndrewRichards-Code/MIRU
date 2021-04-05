#pragma once
namespace miru
{
namespace shader_compiler
{
	const char* help_doucumentation = 
R"(MIRU_SHADER_COMPILER: Help Documentation:
The MIRU_SHADER_COMPILER takes .hlsl file and builds .cso or .spv shader binaries for Direct3D 12 and Vulkan.

-h, -H, -help, -HELP                  : For this help documentation. Optional.
-pause -PAUSE                         : Pauses the program at the end of shader compilation, sets the -h flag. Optional.
-nologo, -NOLOGO                      : Disables copyright message. Optional.
-nooutput, -NOOUTPUT                  : Disables output messages. Optional.
-f:, -F:[filepath]                    : Filepath to a .hlsl file to be compiled. This argument must be set.
-o:, -O:[directory]                   : Directory for the output binaries. Default is the filepath directory.
-i:, -I:[directory]                   : Include directory for compilation. Optional.
-e:, -E:                              : Define an entry point. This argument must be set for non "lib" shader.
-t:, -T:                              : Define the shader model for DXC.
-d, -D                                : Define a marco. MIRU_D3D12 for -cso and MIRU_VULKAN for -spv are already pre-defined.
-cso, -CSO                            : Generates binaries(.cso) for Direct3D 12. This argument or -spv must be set.
-spv, -SPV                            : Generates binaries(.spv) for Vulkan. This argument or -cso must be set.
-dxc:, -DXC:[directory]               : Specify location of the dxc.exe. Optional.
-dxc_args:, -DXC_ARGS:["..."]         : Provide additional arguments directly to dxc. Enclose all arguments in double quotation marks e.g. "Zi -Od".

Supported Shader Models:
 vs_6_0,  vs_6_1,  vs_6_2,  vs_6_3,  vs_6_4,  vs_6_5,  vs_6_6, : Vertex
 hs_6_0,  hs_6_1,  hs_6_2,  hs_6_3,  hs_6_4,  hs_6_5,  hs_6_6, : Hull/Tessellation Control
 ds_6_0,  ds_6_1,  ds_6_2,  ds_6_3,  ds_6_4,  ds_6_5,  ds_6_6, : Domain/Tesselation Evaluation
 gs_6_0,  gs_6_1,  gs_6_2,  gs_6_3,  gs_6_4,  gs_6_5,  gs_6_6, : Geometry
 ps_6_0,  ps_6_1,  ps_6_2,  ps_6_3,  ps_6_4,  ps_6_5,  ps_6_6, : Pixel/Fragment
 cs_6_0,  cs_6_1,  cs_6_2,  cs_6_3,  cs_6_4,  cs_6_5,  cs_6_6, : Compute
         lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6, : Library containing Ray- Generation, Intersection, Any Hit, Closet Hit, Miss and Callable shaders
                                              ms_6_5,  ms_6_6, : Mesh
                                              as_6_5,  as_6_6, : Amplification/Task
)";
}
}