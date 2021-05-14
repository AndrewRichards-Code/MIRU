#include <iostream>
#include <vector>

#include "ErrorCodes.h"
#include "MSCDocumentation.h"
#include "BuildSPV.h"
#include "BuildCSO.h"

#if defined(_WIN64)
#include <Windows.h>
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#define CONSOLE_OUTPUT_RED SetConsoleTextAttribute(hConsole, 4)
#define CONSOLE_OUTPUT_GREEN SetConsoleTextAttribute(hConsole, 2)
#define CONSOLE_OUTPUT_WHITE SetConsoleTextAttribute(hConsole, 7)
#else
#define CONSOLE_OUTPUT_RED
#define CONSOLE_OUTPUT_GREEN
#define CONSOLE_OUTPUT_WHITE
#endif

using namespace miru;
using namespace shader_compiler;

int main(int argc, const char** argv)
{
	MIRU_SHDAER_COMPILER_SET_ERROR_CODE_TO_STRING_FUNCTION;
	
	ErrorCode error = ErrorCode::MIRU_SHADER_COMPILER_OK;

	CONSOLE_OUTPUT_WHITE;

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
		MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: Copyright © 2021 Andrew Richards.\n\n");
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
	std::string filepath, outputDir, entryPoint, dxc_path, glslang_path, shaderStage, shaderModel, dxc_args, glslang_args;
	std::vector<std::string> includeDirs;
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
		else if (tempFilepath.find("-s:") != std::string::npos || tempFilepath.find("-S:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			shaderStage = tempFilepath;
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
		else if (tempFilepath.find("-glslang:") != std::string::npos || tempFilepath.find("-GLSLANG:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-GLSLANG:").size());
			glslang_path = tempFilepath;
		}
		else if (tempFilepath.find("-dxc_args:") != std::string::npos || tempFilepath.find("-DXC_ARGS:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-DXC_ARGS:").size());
			dxc_args = tempFilepath;
		}
		else if (tempFilepath.find("-glslang_args:") != std::string::npos || tempFilepath.find("-GLSLANG_ARGS:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-GLSLANG_ARGS:").size());
			glslang_args = tempFilepath;
		}
		else if (tempFilepath.find("-d") != std::string::npos || tempFilepath.find("-D") != std::string::npos)
		{
			dxc_args += tempFilepath + " ";
			glslang_args += tempFilepath + " ";
		}
		else
		{
			continue;
		}

	}
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
	if (entryPoint.empty())
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_STAGE;
		MIRU_SHADER_COMPILER_RETURN(error, "No shader stage has been passed to MIRU_SHADER_COMPILER.");
	}
	if (shaderStage.empty())
	{
		error = ErrorCode::MIRU_SHADER_COMPILER_NO_SHADER_STAGE;
		MIRU_SHADER_COMPILER_RETURN(error, "No shader stage has been passed to MIRU_SHADER_COMPILER.");
	}
	if (shaderModel.empty())
	{
		shaderModel = "6_0";
	}

	//Build binary
	if (cso)
	{
		CONSOLE_OUTPUT_GREEN;
		error = BuildCSO(filepath, outputDir, includeDirs, entryPoint, shaderStage, shaderModel, dxc_args, dxc_path);
		MIRU_SHADER_COMPILER_ERROR_CODE(error, "CSO Shader Compile Error.\n\n");
		CONSOLE_OUTPUT_WHITE;
	}
	if (spv)
	{
		CONSOLE_OUTPUT_RED;
		error = BuildSPV(filepath, outputDir, includeDirs, entryPoint, shaderStage, glslang_args, glslang_path);
		MIRU_SHADER_COMPILER_ERROR_CODE(error, "SPV Shader Compile Error.\n\n");
		CONSOLE_OUTPUT_WHITE;
	}

	if (pause)
	{
		system("PAUSE");
	}
	MIRU_SHADER_COMPILER_PRINTF("\n");
	MIRU_SHADER_COMPILER_RETURN(error, "MIRU_SHADER_COMPILER returned an error.\n\n");
}