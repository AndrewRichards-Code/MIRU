#include <iostream>
#include <vector>

#include "ErrorCodes.h"
#include "MSCDocumentation.h"
#include "miru_core.h"

using namespace miru;
using namespace shader_compiler;

int main(int argc, const char** argv)
{
	MIRU_SHDAER_COMPILER_SET_ERROR_CODE_TO_STRING_FUNCTION;
	
	ErrorCode error = ErrorCode::MIRU_SHADER_COMPILER_OK;

	//Null arguments
	if (!argc)
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_ARGS;
		MIRU_SHADER_COMPILER_RETURN(error, "No arguments passed to MIRU_SHADER_COMPILER.");
	}

	//Application Header, Help documentation and Debug
	bool logo = true;
	bool pause = false;
	bool help = false;
	for (int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-h") || !_stricmp(argv[i], "-help"))
			help = true;
		if (!_stricmp(argv[i], "-pause"))
			{ pause = true; help = true; }
		if (!_stricmp(argv[i], "-nologo"))
			logo = false;
		if (!_stricmp(argv[i], "-nooutput"))
			output = false;
	}
	if (logo)
		MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: Copyright © 2022 Andrew Richards.\n\n");
	if (help)
	{
		MIRU_SHADER_COMPILER_PRINTF(help_doucumentation);
		MIRU_SHADER_COMPILER_PRINTF("\n");
	}

	//Parse compile flags
	bool cso = false;
	bool spv = false;
	for(int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-cso"))
		{
			cso = true;
			continue;
		}
		if (!_stricmp(argv[i], "-spv"))
		{
			spv = true;
			continue;
		}
	}
	if (!cso && !spv)
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_OUTPUT_FILE_FORMAT;
		MIRU_SHADER_COMPILER_RETURN(error, "No output file formats passed to MIRU_SHADER_COMPILER.");
	}

	//Get Filepath, Directories and others
	std::string filepath, outputDir, entryPoint, shaderModel, dxc_path;
	std::vector<std::string> includeDirs, macros, dxc_args;
	const size_t tagSize = std::string("-X:").size();
	for (int i = 0; i < argc; i++)
	{
		std::string tempFilepath = argv[i];
		if (tempFilepath.find(".hlsl") != std::string::npos && (tempFilepath.find("-f:") != std::string::npos || tempFilepath.find("-F:") != std::string::npos))
		{
			tempFilepath.erase(0, tagSize);
			filepath = tempFilepath;
		}
		else if (tempFilepath.find("-o:") != std::string::npos || tempFilepath.find("-O:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			outputDir = tempFilepath;
		}
		else if (tempFilepath.find("-i:") != std::string::npos || tempFilepath.find("-I:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			includeDirs.push_back(tempFilepath);
		}
		else if (tempFilepath.find("-e:") != std::string::npos || tempFilepath.find("-E:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			entryPoint = tempFilepath;
		}
		else if (tempFilepath.find("-t:") != std::string::npos || tempFilepath.find("-T:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			shaderModel = tempFilepath;
		}
		else if (tempFilepath.find("-dxc:") != std::string::npos || tempFilepath.find("-DXC:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-DXC:").size());
			dxc_path = tempFilepath;
		}
		else if (tempFilepath.find("-dxc_args:") != std::string::npos || tempFilepath.find("-DXC_ARGS:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-DXC_ARGS:").size());
			std::string string;
			std::stringstream stream(tempFilepath);
			while (stream >> string)
				dxc_args.push_back(string);
		}
		else if (tempFilepath.find("-d") != std::string::npos || tempFilepath.find("-D") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize - size_t(1));
			macros.push_back(tempFilepath);
		}
		else
		{
			continue;
		}
	}

	//Check parsed arguments
	if(filepath.empty())
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_FILE;
		MIRU_SHADER_COMPILER_RETURN(error, "No shader has been passed to MIRU_SHADER_COMPILER.");
	}
	if (outputDir.empty())
	{
		size_t fileNamePos = filepath.find_last_of('/');
		outputDir = filepath.substr(0, fileNamePos + 1);
	}
	
	if (shaderModel.empty())
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_MODEL;
		MIRU_SHADER_COMPILER_RETURN(error, "No shader model been passed to MIRU_SHADER_COMPILER.");
	}
	if (entryPoint.empty() && shaderModel.find("lib") == std::string::npos)
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_ENTRY_POINT;
		MIRU_SHADER_COMPILER_RETURN(error, "No entry point has been passed to MIRU_SHADER_COMPILER.");
	}

	//Build binary
	crossplatform::Shader::CompileArguments compileArgs;
	compileArgs.hlslFilepath = filepath;
	compileArgs.outputDirectory = outputDir;
	compileArgs.includeDirectories = includeDirs;
	compileArgs.entryPoint = entryPoint;
	compileArgs.shaderModel = shaderModel;
	compileArgs.macros = macros;
	compileArgs.cso = cso;
	compileArgs.spv = spv;
	compileArgs.dxcArguments = dxc_args;
	compileArgs.dxcLocation = dxc_path;
	crossplatform::Shader::CompileShaderFromSource(compileArgs);

	if (pause)
	{
		system("PAUSE");
	}
	MIRU_SHADER_COMPILER_PRINTF("\n");
	MIRU_SHADER_COMPILER_RETURN(error, "MIRU_SHADER_COMPILER returned an error.\n\n");
}