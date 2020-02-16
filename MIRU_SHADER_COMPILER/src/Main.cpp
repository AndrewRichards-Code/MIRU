#include <iostream>
#include <vector>

#include "ErrorCodes.h"
#include "MSCDocumentation.h"
#include "BuildSPV.h"
#include "BuildCSO.h"

#if defined(_WIN64)
#include <Windows.h>
#endif

using namespace miru;
using namespace shader_compiler;

int main(int argc, const char** argv)
{
	ErrorCode error = ErrorCode::MIRU_SC_OK;

#if defined(_WIN64)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	//Null arguments
	if (!argc)
	{
		error = ErrorCode::MIRU_SC_NO_ARGS;
		MIRU_SHADER_COMPILER_RETURN(error, "No arguements passed to MIRU_SHADER_COMPILER.");
	}

	//Application Header, Help documentation and Debug
	std::cout << "MIRU_SHADER_COMPILER: Copyright � 2020 Andrew Richards.\n\n";
	bool debug = false;
	for (int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-h") || !_stricmp(argv[i], "-help"))
		{
			std::cout << help_doucumentation << std::endl;
		}
		if (!_stricmp(argv[i], "-debug"))
		{
			debug = true;
		}
	}

	//Parse other command line arguements
	bool cso = false;
	bool spv = false;
	bool vi = false;
	bool sr = false;
	bool fo = false;
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
		if (!_stricmp(argv[i], "-vi"))
		{
			vi = true;
			continue;
		}
		if (!_stricmp(argv[i], "-sr"))
		{
			sr = true;
			continue;
		}
		if (!_stricmp(argv[i], "-fo") || !_stricmp(argv[i], "-po"))
		{
			fo = true;
			continue;
		}
	}
	if (!cso && !spv)
	{
		error = ErrorCode::MIRU_SC_NO_OUTPUT_FILE_FORMAT;
		MIRU_SHADER_COMPILER_RETURN(error, "No output file formats passed to MIRU_SHADER_COMPILER.");
	}

	//Get Filepath, Directories and others
	std::string filepath, outputDir, entryPoint, dxc_path, glslang_path, shaderModel, args;
	const size_t tagSize = std::string("-X:").size();
	for (int i = 0; i < argc; i++)
	{
		std::string tempFilepath = argv[i];
		if (tempFilepath.find(".hlsl") != std::string::npos && (tempFilepath.find("-f:") != std::string::npos || tempFilepath.find("-F:") != std::string::npos))
		{
			tempFilepath.erase(0, tagSize);
			filepath = tempFilepath;
		}
		if (tempFilepath.find("-o:") != std::string::npos || tempFilepath.find("-O:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			outputDir = tempFilepath;
		}
		if (tempFilepath.find("-e:") != std::string::npos || tempFilepath.find("-E:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			entryPoint = tempFilepath;
		}
		if (tempFilepath.find("-t:") != std::string::npos || tempFilepath.find("-T:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			shaderModel = tempFilepath;
		}
		if (tempFilepath.find("-dxc:") != std::string::npos || tempFilepath.find("-DXC:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-DXC:").size());
			dxc_path = tempFilepath;
		}
		if (tempFilepath.find("-glslang:") != std::string::npos || tempFilepath.find("-GLSLANG:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-GLSLANG:").size());
			glslang_path = tempFilepath;
		}
		if (tempFilepath.find("-args:") != std::string::npos || tempFilepath.find("-ARGS:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-ARGS:").size());
			args = tempFilepath;
		}
		if (tempFilepath.find("-d") != std::string::npos || tempFilepath.find("-D") != std::string::npos)
		{
			args += tempFilepath + " ";
		}

	}
	if(filepath.empty())
	{
		error = ErrorCode::MIRU_SC_NO_SHADER_FILE;
		MIRU_SHADER_COMPILER_RETURN(error, "No shader has been passed to MIRU_SHADER_COMPILER.");
	}
	if (outputDir.empty())
	{
		size_t fileNamePos = filepath.find_last_of('/');
		outputDir = filepath.substr(0, fileNamePos + 1);
	}
	if (entryPoint.empty())
	{
		entryPoint = "main";
	}
	if (shaderModel.empty())
	{
		shaderModel = "6_0";
	}

	//Build binary
	if (cso)
	{
#if defined(_WIN64)
		SetConsoleTextAttribute(hConsole, 2);
#endif
		error = BuildCSO(filepath, outputDir, entryPoint, shaderModel, args, dxc_path);
		MIRU_SHADER_COMPILER_ERROR_CODE(error, "CSO Shader Compile Error. MIRU_SHADER_COMPILER.");
	}
	if (spv)
	{
#if defined(_WIN64)
		SetConsoleTextAttribute(hConsole, 4);
#endif
		error = BuildSPV(filepath, outputDir, entryPoint, args, glslang_path);
		MIRU_SHADER_COMPILER_ERROR_CODE(error, "SRV Shader Compile Error. MIRU_SHADER_COMPILER.");
	}
#if defined(_WIN64)
	SetConsoleTextAttribute(hConsole, 7);
#endif

	if (debug)
	{
		system("PAUSE");
	}
	MIRU_SHADER_COMPILER_RETURN(error, "Success. MIRU_SHADER_COMPILER.");
}