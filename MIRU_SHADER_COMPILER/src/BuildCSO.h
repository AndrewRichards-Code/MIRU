#pragma once
#include <string>
#include <filesystem>
#include "ErrorCodes.h"

/*OVERVIEW: HLSL Compiler

Version : dxcompiler.dll : 1.4(10.0.18362.1); dxil.dll: 1.4(10.0.18362.1)

USAGE : dxc.exe[options] <inputs>

Common Options :
-help              Display available options
- nologo            Suppress copyright message
- Qunused - arguments Don't emit warning for unused driver arguments

Compilation Options :
-all_resources_bound    Enables agressive flattening
- auto - binding - space <value>
Set auto binding space - enables auto resource binding in libraries
- Cc                     Output color coded assembly listings
- default - linkage <value>
Set default linkage for non - shader functions when compiling or linking to a library target(internal, external)
- denorm <value>         select denormal value options(any, preserve, ftz).any is the default.
- D <value>              Define macro
- enable - 16bit - types     Enable 16bit types and disable min precision types.Available in HLSL 2018 and shader model 6.2
- export - shaders - only    Only export shaders when compiling a library
- exports <value>        Specify exports when compiling a library : export1 [[, export1_clone, ...]= internal_name] [; ...]
- E <value>              Entry point name
- Fc <file>              Output assembly code listing file
- Fd <file>              Write debug information to the given file or directory; trail \ to auto - generate and imply Qstrip_priv
- Fe <file>              Output warnings and errors to the given file
- Fh <file>              Output header file containing object code
- flegacy - macro - expansion
Expand the operands before performing token - pasting operation(fxc behavior)
- flegacy - resource - reservation
Reserve unused explicit register assignments for compatibility with shader model 5.0 and below
- force_rootsig_ver <profile>
force root signature version(rootsig_1_1 if omitted)
- Fo <file>              Output object file
- Gec                    Enable backward compatibility mode
- Ges                    Enable strict mode
- Gfa                    Avoid flow control constructs
- Gfp                    Prefer flow control constructs
- Gis                    Force IEEE strictness
- HV <value>             HLSL version(2016, 2017, 2018).Default is 2018
- H                      Show header includes and nesting depth
- ignore - line - directives Ignore line directives
- I <value>              Add directory to include search path
- Lx                     Output hexadecimal literals
- Ni                     Output instruction numbers in assembly listings
- no - warnings            Suppress warnings
- not_use_legacy_cbuf_load
Do not use legacy cbuffer load
- No                     Output instruction byte offsets in assembly listings
- Odump                  Print the optimizer commands.
- Od                     Disable optimizations
- pack_optimized         Optimize signature packing assuming identical signature provided for each connecting stage
- pack_prefix_stable(default) Pack signatures preserving prefix - stable property - appended elements will not disturb placement of prior elements
- recompile              recompile from DXIL container with Debug Info or Debug Info bitcode file
- res_may_alias          Assume that UAVs / SRVs may alias
- rootsig - define <value> Read root signature from a #define
- T <profile>            Set target profile.
<profile>: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4,
vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4,
cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4,
gs_6_0, gs_6_1, gs_6_2, gs_6_3, gs_6_4,
ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4,
hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4,
lib_6_3, lib_6_4
- Vd                     Disable validation
- Vi                     Display details about the include process.
- Vn <name>              Use <name> as variable name in header file
- WX                     Treat warnings as errors
- Zi                     Enable debug information
- Zpc                    Pack matrices in column - major order
- Zpr                    Pack matrices in row - major order
- Zsb                    Build debug name considering only output binary
- Zss                    Build debug name considering source information

Optimization Options :
-O0 Optimization Level 0
- O1 Optimization Level 1
- O2 Optimization Level 2
- O3 Optimization Level 3 (Default)

SPIR - V CodeGen Options :
-fspv - debug = <value>     Specify whitelist of debug info category(file->source->line, tool)
- fspv - extension = <value> Specify SPIR - V extension permitted to use
- fspv - reflect           Emit additional SPIR - V instructions to aid reflection
- fspv - target - env = <value>
Specify the target environment : vulkan1.0 (default) or vulkan1.1
- fvk - b - shift <shift> <space>
Specify Vulkan binding number shift for b - type register
- fvk - bind - register <type - number> <space> <binding> <set>
Specify Vulkan descriptor setand binding for a specific register
- fvk - invert - y           Negate SV_Position.y before writing to stage output in VS / DS / GS to accommodate Vulkan's coordinate system
- fvk - s - shift <shift> <space>
Specify Vulkan binding number shift for s - type register
- fvk - t - shift <shift> <space>
Specify Vulkan binding number shift for t - type register
- fvk - u - shift <shift> <space>
Specify Vulkan binding number shift for u - type register
- fvk - use - dx - layout      Use DirectX memory layout for Vulkan resources
- fvk - use - dx - position - w  Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
- fvk - use - gl - layout      Use strict OpenGL std140 / std430 memory layout for Vulkan resources
- fvk - use - scalar - layout  Use scalar memory layout for Vulkan resources
- Oconfig = <value>        Specify a comma - separated list of SPIRV - Tools passes to customize optimization configuration(see http ://khr.io/hlsl2spirv#optimization)
-spirv                  Generate SPIR - V code

Utility Options :
-dumpbin              Load a binary file rather than compiling
- extractrootsignature Extract root signature from shader bytecode(must be used with / Fo <file>)
- getprivate <file>    Save private data from shader blob
- P <value>            Preprocess to file(must be used alone)
- Qstrip_debug         Strip debug information from 4_0 + shader bytecode(must be used with / Fo <file>)
- Qstrip_priv          Strip private data from shader bytecode(must be used with / Fo <file>)
- Qstrip_reflect       Strip reflection data from shader bytecode(must be used with / Fo <file>)
- Qstrip_rootsignature Strip root signature data from shader bytecode(must be used with / Fo <file>)
- setprivate <file>    Private data to add to compiled shader blob
- setrootsignature <file>
Attach root signature to shader bytecode
- verifyrootsignature <file>
Verify shader bytecode with root signature*/

namespace miru
{
namespace shader_compiler
{
	//Use Relative paths and do not use preceeding or trailing '/'. see Example.
	//Output file will have ".spv" append to the end automatically to denoted that it's a SPIR-V file.
	//Example miru::shader_compiler::BuildCSO("res/shaders/basic.vert.hlsl", "res/bin");
	ErrorCode BuildCSO(const std::string& filepath, const std::string& outputDirectory, const std::string& entryPoint, const std::string& additionCommandlineArgs = "", const std::string& compiler_dir = "")
	{
		//Find Get DXC Directory
	#if _WIN64
		std::string binDir = "\\x64";
	#elif _WIN32
		std::string binDir = "\\x86";
	#endif

		std::string winKitSDKDir = "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0";
		std::string dxcLocation = winKitSDKDir + binDir;

		if (!compiler_dir.empty())
			dxcLocation = compiler_dir;

		size_t fileNamePos = filepath.find_last_of('/');
		size_t hlslExtPos = filepath.find(".hlsl");
		std::string filename = filepath.substr(fileNamePos, hlslExtPos - fileNamePos);

		std::string currentWorkingDir = std::filesystem::current_path().string();
		if (currentWorkingDir.find("exe") != std::string::npos)
			currentWorkingDir += "/../../..";
		std::string absoluteSrcDir = /*currentWorkingDir + "/" +*/ filepath;
		std::string absoluteDstDir = /*currentWorkingDir + "/" +*/ outputDirectory + filename + ".cso";

		std::string shaderType;
		{
			if (filepath.find(".vert") != std::string::npos) { shaderType = "vs_"; }
			if (filepath.find(".tesc") != std::string::npos) { shaderType = "hs_"; }
			if (filepath.find(".tese") != std::string::npos) { shaderType = "ds_"; }
			if (filepath.find(".geom") != std::string::npos) { shaderType = "gs_"; }
			if (filepath.find(".frag") != std::string::npos) { shaderType = "ps_"; }
			if (filepath.find(".comp") != std::string::npos) { shaderType = "cs_"; }
		}
		const std::string shaderModel = "6_0";

		std::string command = "dxc -T " + shaderType + shaderModel + " -E " + entryPoint + " " + absoluteSrcDir + " -Fo " + absoluteDstDir + additionCommandlineArgs;

		//Run dxc
		printf("MIRU_SHADER_COMPILER: HLSL -> CSO using DXC\n");
		printf(("Executing: " + dxcLocation + "> " + command + "\n").c_str());
		int errorCode = system(("cd " + dxcLocation + " && " + command).c_str());
		printf("\n");

		if (errorCode)
			return ErrorCode::MIRU_SC_CSO_ERROR;
		else
			return ErrorCode::MIRU_SC_OK;
	}

	static void ClearConsoleScreenCSO()
	{
		system("CLS");
	}
}
}