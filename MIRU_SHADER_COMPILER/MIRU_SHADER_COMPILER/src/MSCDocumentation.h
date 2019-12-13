#pragma once
namespace miru
{
namespace shader_compiler
{
	const char* help_doucumentation = 
R"(MIRU_SHADER_COMPILER Help Documentation:
-h, -H, -help, -HELP        : For this help documentation.
-f:, -F:[filepath]          : Filepath to a .msf file to be compiled.
-o:, -O:[directory]         : Directory for the output binaries.
-i:, -I:[directory]         : Directory for the intermediary hlsl/glsl files.
-hlsl, -HLSL                : Generates hlsl files and binaries(.cso) for Direct3D 12.
-glsl, -GLSL                : Generates glsl files and binaries(.spv) for Vulkan.
-hlsl_no_bin, -HLSL_NO_BIN  : Disables automatic binary generation for hlsl.
-glsl_no_bin, -GLSL_NO_BIN  : Disables automatic binary generation for glsl.
-hlsl_no_src, -HLSL_NO_SRC  : Generated hlsl files are not saved to disk.
-glsl_no_src, -GLSL_NO_SRC  : Generated glsl files are not saved to disk.
-vi, -VI                    : Generates Vertex Input Info.
-sr, -SR                    : Generates Shader Resources Info.
-fo, -FO, -po, -PO          : Generates Fragment/Pixel Output Info.
)";
}
}