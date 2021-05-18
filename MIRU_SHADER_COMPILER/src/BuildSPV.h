#pragma once
#include <string>
#include <fstream>
#include "ErrorCodes.h"

namespace miru
{
namespace shader_compiler
{
	//Use Absolute paths and do not use preceeding or trailing '/'. see Example.
	//Output file will have ".spv" append to the end automatically to denoted that it's a SPIR-V file.
	//Example miru::shader_compiler::BuildSPV("$(ProjectDir)res/shaders/basic.vert.hlsl", "$(ProjectDir)res/bin", "main");
	ErrorCode BuildSPV(
		const std::string& filepath,
		const std::string& outputDirectory,
		const std::vector<std::string>& includeDirectories,
		const std::string& entryPoint,
		const std::string& shaderModel,
		const std::string& additionCommandlineArgs,
		const std::string& compiler_dir)
	{
		//Find Vulkan SDK Directory
#if _WIN64
		std::string binDir = "/Bin";
#elif __linux__ && (__x86_64__ || _M_X64)
		std::string binDir = "/x86_64/bin";
#endif

#if !(_CRT_SECURE_NO_WARNINGS) && (_WIN32)
		char* buffer = nullptr;
		size_t size = 0;
		_dupenv_s(&buffer, &size, "VULKAN_SDK");
		std::string vulkanSDKDir = (buffer != nullptr) ? std::string(buffer) : std::string("");
#else
		std::string vulkanSDKDir = std::getenv("VULKAN_SDK");
#endif

		std::string dxcLocation = vulkanSDKDir + binDir;

		if (!compiler_dir.empty())
			dxcLocation = compiler_dir;

		size_t fileNamePos = filepath.find_last_of('/');
		size_t hlslExtPos = filepath.find(".hlsl");
		std::string filename = filepath.substr(fileNamePos, hlslExtPos - fileNamePos);

		std::string absoluteSrcDir = filepath;
		std::string absoluteDstDir = outputDirectory + filename + "_" + shaderModel;
		if (!entryPoint.empty())
			absoluteDstDir += "_" + entryPoint;
		absoluteDstDir += ".spv";

		std::string command = "dxc -spirv -fspv-target-env=vulkan1.2";
		command += " -T " + shaderModel;
		if (!entryPoint.empty())
			command += " -E " + entryPoint;
		command += " " + absoluteSrcDir;
		command += " -Fo " + absoluteDstDir;
		for (auto& includeDirectory : includeDirectories)
			command += " -I " + includeDirectory;
		command += " -DMIRU_D3D12 " + additionCommandlineArgs;

		//Run glslangValidator
		MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: HLSL -> SPV using DXC\n");
		MIRU_SHADER_COMPILER_PRINTF(("Executing: " + dxcLocation + "> " + command + "\n").c_str());
		int errorCode = system(("cd " + dxcLocation + " && " + command).c_str());
		MIRU_SHADER_COMPILER_PRINTF("'dxc.exe' has exited with code %d (0x%x).\n", errorCode, errorCode);
		MIRU_SHADER_COMPILER_PRINTF("MIRU_SHADER_COMPILER: HLSL -> SPV finished.\n\n");

		if (errorCode)
		{
			return ErrorCode::MIRU_SHADER_COMPILER_SPV_ERROR;
		}
		
		return ErrorCode::MIRU_SHADER_COMPILER_OK;
	}

	void ClearConsoleScreenSPV()
	{
		system("CLS");
	}
}
}
