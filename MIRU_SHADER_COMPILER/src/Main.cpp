#include "ErrorCodes.h"
#include "MSCDocumentation.h"
#include "miru_core.h"

using namespace miru;
using namespace shader_compiler;

bool FoundArg(const std::string& arg, const std::string& value)
{
	return (arg.find(value) != std::string::npos);
}

void BuildBinary(base::Shader::CompileArguments compileArgs)
{
	base::Shader::CompileShaderFromSource(compileArgs);
}

int main(int argc, const char** argv)
{
	MIRU_SHDAER_COMPILER_SET_ERROR_CODE_TO_STRING_FUNCTION;
	
	ErrorCode error = ErrorCode::MIRU_SHADER_COMPILER_OK;

	//Null arguments
	if (argc == 1)
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
		if (FoundArg(argv[i], "-h") || FoundArg(argv[i], "-H") || FoundArg(argv[i], "-help") || FoundArg(argv[i], "-HELP"))
			help = true;
		if (FoundArg(argv[i], "-pause") || FoundArg(argv[i], "-PAUSE"))
			{ pause = true; help = true; }
		if (FoundArg(argv[i], "-nologo") || FoundArg(argv[i], "-NOLOGO"))
			logo = false;
		if (FoundArg(argv[i], "-nooutput") || FoundArg(argv[i], "-NOOUTPUT"))
			output = false;
	}
	if (logo)
		MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: Copyright © 2019-2023 Andrew Richards.\n\n");
	if (help)
	{
		MIRU_SHADER_COMPILER_PRINTF(help_doucumentation);
		MIRU_SHADER_COMPILER_PRINTF("\n");
	}

	//Check for Recompile Argument Files
	std::unordered_map<std::string, std::string> rafEnvironmentVariables;
	for (int i = 0; i < argc; i++)
	{
		const size_t tagSize = std::string("-RAFD:").size();
		std::string arg = argv[i];
		if (FoundArg(arg, "-rafd:") || FoundArg(arg, "-RAFD:"))
		{
			arg.erase(0, tagSize);
			if (FoundArg(arg, "="))
			{
				std::stringstream ss = std::stringstream(arg);
				std::string regexVariable;
				std::string value;
				std::getline(ss, regexVariable, '=');
				std::getline(ss, value, '=');
				rafEnvironmentVariables.insert({ regexVariable, value });
			}
		}
	}

	bool raf = false;
	for (int i = 0; i < argc; i++)
	{
		const size_t tagSize = std::string("-RAF:").size();
		std::string arg = argv[i];

		if (FoundArg(arg, "-raf:") || FoundArg(arg, "-RAF:") || raf)
		{
			raf = true;
			if (FoundArg(arg, "-raf:") || FoundArg(arg, "-RAF:"))
				arg.erase(0, tagSize);

			if (FoundArg(arg, ".json"))
			{
				std::filesystem::path filepath = arg;
				const std::vector<base::Shader::CompileArguments> compileArguments 
					= base::Shader::LoadCompileArgumentsFromFile(
					filepath, rafEnvironmentVariables);

				for (const auto& compileArgument : compileArguments)
				{
					BuildBinary(compileArgument);
				}
			}
		}
		else 
		{
			continue;
		}

	}
	if (raf)
	{
		return 0;
	}

	//Parse compile flags
	bool cso = false;
	bool spv = false;
	for(int i = 0; i < argc; i++)
	{
		if (FoundArg(argv[i], "-cso") || FoundArg(argv[i], "-CSO"))
		{
			cso = true;
			continue;
		}
		if (FoundArg(argv[i], "-spv") || FoundArg(argv[i], "-SPV"))
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
	std::string filepath, outputDir, entryPoint, shaderModel;
	std::vector<std::string> includeDirs, macros, dxc_args;
	const size_t tagSize = std::string("-X:").size();
	for (int i = 0; i < argc; i++)
	{
		std::string arg = argv[i];
		if (FoundArg(arg, ".hlsl") && (FoundArg(arg, "-f:") || FoundArg(arg, "-F:")))
		{
			arg.erase(0, tagSize);
			filepath = arg;
		}
		else if (FoundArg(arg, "-o:") || FoundArg(arg, "-O:"))
		{
			arg.erase(0, tagSize);
			outputDir = arg;
		}
		else if (FoundArg(arg, "-i:") || FoundArg(arg, "-I:"))
		{
			arg.erase(0, tagSize);
			includeDirs.push_back(arg);
		}
		else if (FoundArg(arg, "-e:") || FoundArg(arg, "-E:"))
		{
			arg.erase(0, tagSize);
			entryPoint = arg;
		}
		else if (FoundArg(arg, "-t:") || FoundArg(arg, "-T:"))
		{
			arg.erase(0, tagSize);
			shaderModel = arg;
		}
		else if (FoundArg(arg, "-dxc_args:") || FoundArg(arg, "-DXC_ARGS:"))
		{
			arg.erase(0, std::string("-DXC_ARGS:").size());
			std::string string;
			std::stringstream stream(arg);
			while (stream >> string)
				dxc_args.push_back(string);
		}
		else if (FoundArg(arg, "-d:") || FoundArg(arg, "-D:"))
		{
			arg.erase(0, tagSize);
			macros.push_back(arg);
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
	base::Shader::CompileArguments compileArgs;
	compileArgs.hlslFilepath = filepath;
	compileArgs.outputDirectory = outputDir;
	compileArgs.includeDirectories = includeDirs;
	compileArgs.entryPoint = entryPoint;
	compileArgs.shaderModel = shaderModel;
	compileArgs.macros = macros;
	compileArgs.cso = cso;
	compileArgs.spv = spv;
	compileArgs.dxcArguments = dxc_args;
	BuildBinary(compileArgs);

	if (pause)
	{
		system("PAUSE");
	}
	MIRU_SHADER_COMPILER_PRINTF("\n");
	MIRU_SHADER_COMPILER_RETURN(error, "MIRU_SHADER_COMPILER returned an error.\n\n");
}