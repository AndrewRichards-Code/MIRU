#pragma once
#include "miru_shader_core_common.h"

namespace miru
{
namespace shader_core
{
	//See MSCDocumentation.h for correct usage.
	//All filepaths and directories must be relative to the current working directory.
	//All locations must be full paths i.e. dxc.
	struct CompileArguments
	{
		std::string					hlslFilepath;
		std::string					outputDirectory;	//Optional
		std::vector<std::string>	includeDirectories;	//Optional
		std::string					entryPoint;			//Required for non lib shaders.
		std::string					shaderModel;		//Optional
		std::vector<std::string>	macros;				//Optional
		bool						cso;				//Either cso or spv must be true
		bool						spv;				//Either cso or spv must be true
		std::vector<std::string>	dxcArguments;		//Optional
		std::string					dxcLocation;		//Optional
		bool						nologo;				//Optional
		bool						nooutput;			//Optional
	};
}
}