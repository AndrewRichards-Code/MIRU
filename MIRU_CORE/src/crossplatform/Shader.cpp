#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "directx12/D3D12Shader.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKShader.h"
#endif

#include "ARC/src/FileSystemHelpers.h"
#include "ARC/src/FileLoader.h"

using namespace miru;
using namespace crossplatform;

Ref<Shader> Shader::Create(Shader::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Shader>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Shader>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

void Shader::Recompile()
{
	MIRU_CPU_PROFILE_FUNCTION();

	#if !defined(MIRU_WIN64_UWP)
	int returnCode = Call_MIRU_SHADER_COMPILER();
	if (returnCode != 0)
	{
		MIRU_WARN(returnCode, "WARN: CROSSPLATFORM: MIRU_SHADER_COMIPLER returned an error.");
	}
	else
	{
		Reconstruct();
	}
	#endif
}

void Shader::GetShaderByteCode()
{
	MIRU_CPU_PROFILE_FUNCTION();

	//Check for valid shader source
	std::string binFilepath;
	if(!m_CI.binaryFilepath.empty())
		binFilepath = std::string(m_CI.binaryFilepath);

	if (binFilepath.empty() && m_CI.binaryCode.empty())
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: No file path or binary code provided.");

	//Load from binary code if no binary filapath is provided
	if (binFilepath.empty() && !m_CI.binaryCode.empty())
	{
		m_ShaderBinary = m_CI.binaryCode;
		return;
	}

	//Load from binary filepath
	std::string shaderBinaryFileExtension;
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		shaderBinaryFileExtension = ".cso"; break;
	case GraphicsAPI::API::VULKAN:
		shaderBinaryFileExtension = ".spv"; break;
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return;
	}
	
	binFilepath = binFilepath.replace(binFilepath.find_last_of('.'), 4, shaderBinaryFileExtension);

	//Check shader exist and the binary is upto date.
	#if !defined(MIRU_WIN64_UWP)
	bool buildShader = true;
	if (arc::FileExist(binFilepath) && arc::FileExist(m_CI.recompileArguments.hlslFilepath))
	{
		buildShader = arc::FileLastWriteTime(binFilepath) < arc::FileLastWriteTime(m_CI.recompileArguments.hlslFilepath);
	}

	if (buildShader)
	{
		int returnCode = Call_MIRU_SHADER_COMPILER();
		if (returnCode != 0)
		{
			MIRU_ASSERT(returnCode, "ERROR: CROSSPLATFORM: MIRU_SHADER_COMIPLER returned an error.");
		}
	}
	#endif

	m_ShaderBinary = arc::ReadBinaryFile(binFilepath);
	MIRU_ASSERT(m_ShaderBinary.empty(), "ERROR: CROSSPLATFORM: Unable to read shader binary file.");
}

int Shader::Call_MIRU_SHADER_COMPILER()
{
	#if defined(__ANDROID__)
	MIRU_WARN(true, "WARN: CROSSPLATFORM: MIRU_SHADER_COMPILER is not available on Android. Use offline shader compliation.");
	return 0;
	#endif

	if (m_CI.recompileArguments.hlslFilepath.empty()
		&& m_CI.recompileArguments.mscDirectory.empty()
		&& (m_CI.recompileArguments.cso || m_CI.recompileArguments.spv))
	{
		MIRU_WARN(true, "WARN: CROSSPLATFORM: Invalid recompile arguments provided.");
		return 1;
	}

	std::string currentWorkingDir = std::filesystem::current_path().string() + "\\";

	std::string command = "MIRU_SHADER_COMPILER ";
	command += " -f:" + currentWorkingDir + m_CI.recompileArguments.hlslFilepath;
	command += " -o:" + currentWorkingDir + m_CI.recompileArguments.outputDirectory;
	for (auto& includeDir : m_CI.recompileArguments.includeDirectories)
		command += " -i:" + currentWorkingDir + includeDir;
	if (!m_CI.recompileArguments.entryPoint.empty())
		command += " -e:" + m_CI.recompileArguments.entryPoint;
	if (!m_CI.recompileArguments.shaderStage.empty())
		command += " -s:" + m_CI.recompileArguments.shaderStage;
	if (!m_CI.recompileArguments.shaderModel.empty())
		command += " -t:" + m_CI.recompileArguments.shaderModel;
	for (auto& macro : m_CI.recompileArguments.macros)
		command += " -d:" + std::string(macro);
	if (m_CI.recompileArguments.cso)
		command += " -cso";
	if (m_CI.recompileArguments.spv)
		command += " -spv";
	if (!m_CI.recompileArguments.dxcLocation.empty())
		command += " -dxc:" + m_CI.recompileArguments.dxcLocation;
	if (!m_CI.recompileArguments.glslangLocation.empty())
		command += " -glslang:" + m_CI.recompileArguments.glslangLocation;
	if (!m_CI.recompileArguments.dxcArguments.empty())
		command += " -dxc_args:" + m_CI.recompileArguments.dxcArguments;
	if (!m_CI.recompileArguments.glslangArguments.empty())
		command += " -glslang_args:" + m_CI.recompileArguments.glslangArguments;
	if (m_CI.recompileArguments.nologo)
		command += " -nologo";
	if (m_CI.recompileArguments.nooutput)
		command += " -nooutput";

	std::string mscLocation = currentWorkingDir + m_CI.recompileArguments.mscDirectory;

	MIRU_PRINTF("%s", ("MIRU_CORE: Recompiling shader: " + currentWorkingDir + m_CI.recompileArguments.hlslFilepath + "\n").c_str());
	MIRU_PRINTF("%s", ("Executing: " + mscLocation + "> " + command + "\n\n").c_str());
	int returnCode = system(("cd " + mscLocation + " && " + command).c_str());
	MIRU_PRINTF("'MIRU_SHADER_COMPILER.exe' has exited with code %d (0x%x).\n", returnCode, returnCode);
	MIRU_PRINTF("%s", "MIRU_CORE: Recompiling shader finished.\n\n");

	return returnCode;
}