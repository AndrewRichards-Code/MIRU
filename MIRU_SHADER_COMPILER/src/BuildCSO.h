#pragma once
#include <string>
#include "ErrorCodes.h"

/*OVERVIEW: HLSL Compiler for Windows

Version: dxcompiler.dll: 1.6 - 1.5.0.2891 (cc43bc82)

USAGE: dxc.exe [options] <inputs>

Common Options:
  -help              Display available options
  -nologo            Suppress copyright message
  -Qunused-arguments Don't emit warning for unused driver arguments

Compilation Options:
  -all-resources-bound    Enables agressive flattening
  -auto-binding-space <value>
                          Set auto binding space - enables auto resource binding in libraries
  -Cc                     Output color coded assembly listings
  -default-linkage <value>
                          Set default linkage for non-shader functions when compiling or linking to a library target (internal, external)
  -denorm <value>         select denormal value options (any, preserve, ftz). any is the default.
  -D <value>              Define macro
  -enable-16bit-types     Enable 16bit types and disable min precision types. Available in HLSL 2018 and shader model 6.2
  -enable-lifetime-markers
                          Enable generation of lifetime markers
  -encoding <value>       Set default encoding for text outputs (utf8|utf16) default=utf8
  -export-shaders-only    Only export shaders when compiling a library
  -exports <value>        Specify exports when compiling a library: export1[[,export1_clone,...]=internal_name][;...]
  -E <value>              Entry point name
  -Fc <file>              Output assembly code listing file
  -fdiagnostics-show-option
                          Print option name with mappable diagnostics
  -Fd <file>              Write debug information to the given file, or automatically named file in directory when ending in '\'
  -Fe <file>              Output warnings and errors to the given file
  -Fh <file>              Output header file containing object code
  -flegacy-macro-expansion
                          Expand the operands before performing token-pasting operation (fxc behavior)
  -flegacy-resource-reservation
                          Reserve unused explicit register assignments for compatibility with shader model 5.0 and below
  -fno-diagnostics-show-option
                          Do not print option name with mappable diagnostics
  -force-rootsig-ver <profile>
                          force root signature version (rootsig_1_1 if omitted)
  -Fo <file>              Output object file
  -Fre <file>             Output reflection to the given file
  -Frs <file>             Output root signature to the given file
  -Fsh <file>             Output shader hash to the given file
  -Gec                    Enable backward compatibility mode
  -Ges                    Enable strict mode
  -Gfa                    Avoid flow control constructs
  -Gfp                    Prefer flow control constructs
  -Gis                    Force IEEE strictness
  -HV <value>             HLSL version (2016, 2017, 2018). Default is 2018
  -H                      Show header includes and nesting depth
  -ignore-line-directives Ignore line directives
  -I <value>              Add directory to include search path
  -Lx                     Output hexadecimal literals
  -Ni                     Output instruction numbers in assembly listings
  -no-legacy-cbuf-layout  Do not use legacy cbuffer load
  -no-warnings            Suppress warnings
  -No                     Output instruction byte offsets in assembly listings
  -Odump                  Print the optimizer commands.
  -Od                     Disable optimizations
  -pack-optimized         Optimize signature packing assuming identical signature provided for each connecting stage
  -pack-prefix-stable     (default) Pack signatures preserving prefix-stable property - appended elements will not disturb placement of prior elements
  -recompile              recompile from DXIL container with Debug Info or Debug Info bitcode file
  -res-may-alias          Assume that UAVs/SRVs may alias
  -rootsig-define <value> Read root signature from a #define
  -T <profile>            Set target profile.
        <profile>: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, ps_6_5, ps_6_6,
                 vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4, vs_6_5, vs_6_6,
                 gs_6_0, gs_6_1, gs_6_2, gs_6_3, gs_6_4, gs_6_5, gs_6_6,
                 hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6,
                 ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6,
                 cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6,
                 lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6,
                 ms_6_5, ms_6_6,
                 as_6_5, as_6_6,

  -Vd                     Disable validation
  -Vi                     Display details about the include process.
  -Vn <name>              Use <name> as variable name in header file
  -WX                     Treat warnings as errors
  -Zi                     Enable debug information
  -Zpc                    Pack matrices in column-major order
  -Zpr                    Pack matrices in row-major order
  -Zsb                    Compute Shader Hash considering only output binary
  -Zss                    Compute Shader Hash considering source information

Optimization Options:
  -O0 Optimization Level 0
  -O1 Optimization Level 1
  -O2 Optimization Level 2
  -O3 Optimization Level 3 (Default)

Rewriter Options:
  -extract-entry-uniforms Move uniform parameters from entry point to global scope
  -global-extern-by-default
                          Set extern on non-static globals
  -keep-user-macro        Write out user defines after rewritten HLSL
  -line-directive         Add line directive
  -remove-unused-functions
                          Remove unused functions and types
  -remove-unused-globals  Remove unused static globals and functions
  -skip-fn-body           Translate function definitions to declarations
  -skip-static            Remove static functions and globals when used with -skip-fn-body
  -unchanged              Rewrite HLSL, without changes.

SPIR-V CodeGen Options:
  -fspv-debug=<value>     Specify whitelist of debug info category (file -> source -> line, tool)
  -fspv-extension=<value> Specify SPIR-V extension permitted to use
  -fspv-flatten-resource-arrays
                          Flatten arrays of resources so each array element takes one binding number
  -fspv-reflect           Emit additional SPIR-V instructions to aid reflection
  -fspv-target-env=<value>
                          Specify the target environment: vulkan1.0 (default) or vulkan1.1
  -fvk-auto-shift-bindings
                          Apply fvk-*-shift to resources without an explicit register assignment.
  -fvk-b-shift <shift> <space>
                          Specify Vulkan binding number shift for b-type register
  -fvk-bind-globals <binding> <set>
                          Specify Vulkan binding number and set number for the $Globals cbuffer
  -fvk-bind-register <type-number> <space> <binding> <set>
                          Specify Vulkan descriptor set and binding for a specific register
  -fvk-invert-y           Negate SV_Position.y before writing to stage output in VS/DS/GS to accommodate Vulkan's coordinate system
  -fvk-s-shift <shift> <space>
                          Specify Vulkan binding number shift for s-type register
  -fvk-t-shift <shift> <space>
                          Specify Vulkan binding number shift for t-type register
  -fvk-u-shift <shift> <space>
                          Specify Vulkan binding number shift for u-type register
  -fvk-use-dx-layout      Use DirectX memory layout for Vulkan resources
  -fvk-use-dx-position-w  Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
  -fvk-use-gl-layout      Use strict OpenGL std140/std430 memory layout for Vulkan resources
  -fvk-use-scalar-layout  Use scalar memory layout for Vulkan resources
  -Oconfig=<value>        Specify a comma-separated list of SPIRV-Tools passes to customize optimization configuration (see http://khr.io/hlsl2spirv#optimization)
  -spirv                  Generate SPIR-V code

Utility Options:
  -dumpbin              Load a binary file rather than compiling
  -extractrootsignature Extract root signature from shader bytecode (must be used with /Fo <file>)
  -getprivate <file>    Save private data from shader blob
  -P <value>            Preprocess to file (must be used alone)
  -Qembed_debug         Embed PDB in shader container (must be used with /Zi)
  -Qstrip_debug         Strip debug information from 4_0+ shader bytecode  (must be used with /Fo <file>)
  -Qstrip_priv          Strip private data from shader bytecode  (must be used with /Fo <file>)
  -Qstrip_reflect       Strip reflection data from shader bytecode  (must be used with /Fo <file>)
  -Qstrip_rootsignature Strip root signature data from shader bytecode  (must be used with /Fo <file>)
  -setprivate <file>    Private data to add to compiled shader blob
  -setrootsignature <file>
                        Attach root signature to shader bytecode
  -verifyrootsignature <file>
                        Verify shader bytecode with root signature

Warning Options:
  -W[no-]<warning> Enable/Disable the specified warning*/

namespace miru
{
namespace shader_compiler
{
	//Use Absolute paths and do not use preceeding or trailing '/'.
	//Output file will have ".cso" append to the end automatically to denoted that it's a CSO file.
	ErrorCode BuildCSO(
		const std::string& filepath, 
		const std::string& outputDirectory, 
		const std::vector<std::string>& includeDirectories, 
		const std::string& entryPoint, 
		const std::string& shaderModel, 
		const std::string& additionCommandlineArgs, 
		const std::string& compiler_dir)
	{
		//Find Get DXC Directory
		std::string dxcLocation = "C:/Program Files (x86)/Windows Kits/10/bin/" + std::string(WIN_TARGET_PLATFORM_VERSION) + "/x64";

		if (!compiler_dir.empty())
			dxcLocation = compiler_dir;

		size_t fileNamePos = filepath.find_last_of('/');
		size_t hlslExtPos = filepath.find(".hlsl");
		std::string filename = filepath.substr(fileNamePos, hlslExtPos - fileNamePos);

		std::string absoluteSrcDir = filepath;
		std::string absoluteDstDir = outputDirectory + filename + "_" + shaderModel;
		if(!entryPoint.empty())
			absoluteDstDir += "_" + entryPoint;
		absoluteDstDir += ".cso";

		int errorCode = 0;
		if (shaderModel.find("5_1") != std::string::npos)
		{
			std::string command = "fxc";
			command += " -T " + shaderModel;
			if (!entryPoint.empty())
				command += " -E " + entryPoint;
			command += " " + absoluteSrcDir;
			command += " -Fo " + absoluteDstDir;
			for (auto& includeDirectory : includeDirectories)
				command += " -I " + includeDirectory;
			command += " -DMIRU_D3D12 " + additionCommandlineArgs;

			//Run fxc
			MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: HLSL -> CSO using FXC\n");
			MIRU_SHADER_COMPILER_PRINTF(("Executing: " + dxcLocation + "> " + command + "\n").c_str());
			errorCode = system(("cd " + dxcLocation + " && " + command).c_str());
			MIRU_SHADER_COMPILER_PRINTF("'fxc.exe' has exited with code %d (0x%x).\n", errorCode, errorCode);
			MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: HLSL -> CSO finished.\n\n");
		}
		else
		{
			std::string command = "dxc";
			command += " -T " + shaderModel;
			if (!entryPoint.empty())
				command += " -E " + entryPoint;
			command += " " + absoluteSrcDir;
			command += " -Fo " + absoluteDstDir;
			for (auto& includeDirectory : includeDirectories)
				command += " -I " + includeDirectory;
			command += " -DMIRU_D3D12 " + additionCommandlineArgs;

			//Run dxc
			MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: HLSL -> CSO using DXC\n");
			MIRU_SHADER_COMPILER_PRINTF(("Executing: " + dxcLocation + "> " + command + "\n").c_str());
			errorCode = system(("cd " + dxcLocation + " && " + command).c_str());
			MIRU_SHADER_COMPILER_PRINTF("'dxc.exe' has exited with code %d (0x%x).\n", errorCode, errorCode);
			MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: HLSL -> CSO finished.\n\n");
		}

		if (errorCode)
			return ErrorCode::MIRU_SHADER_COMPILER_CSO_ERROR;
		else
			return ErrorCode::MIRU_SHADER_COMPILER_OK;
	}

	static void ClearConsoleScreenCSO()
	{
		system("CLS");
	}
}
}