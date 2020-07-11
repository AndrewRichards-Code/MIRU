#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "directx12/D3D12Shader.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKShader.h"
#endif

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

void Shader::GetShaderByteCode()
{
	MIRU_CPU_PROFILE_FUNCTION();

	//Check for valid shader source
	std::string binFilepath;
	if(m_CI.binaryFilepath)
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

	std::ifstream stream(binFilepath, std::ios::ate | std::ios::binary);
	if (!stream.is_open())
	{
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unable to read shader binary file.");
	}
	m_ShaderBinary.resize(static_cast<size_t>(stream.tellg()));
	stream.seekg(0);
	stream.read(m_ShaderBinary.data(), static_cast<std::streamsize>(m_ShaderBinary.size()));
	stream.close();
}

void Shader::Recompile()
{
#if defined(__ANDROID__)
    return;
#endif
	MIRU_CPU_PROFILE_FUNCTION();

	if (!m_CI.recompileArguments.hlslFilepath 
		&& !m_CI.recompileArguments.mscDirectory
		&& (m_CI.recompileArguments.cso || m_CI.recompileArguments.spv))
	{
		MIRU_WARN(true, "WARN: CROSSPLATFORM: Invalid recompile arguments provided.");
		return;
	}

	#ifndef __cpp_lib_filesystem
	std::string currentWorkingDir = std::experimental::filesystem::current_path().string() + "/";
	#else
	std::string currentWorkingDir = std::filesystem::current_path().string() + "/";
	#endif

	std::string command = "MIRU_SHADER_COMPILER ";
	command += " -f:" + currentWorkingDir + m_CI.recompileArguments.hlslFilepath;
	command += " -o:" + currentWorkingDir + m_CI.recompileArguments.outputDirectory;
	for(auto& includeDir : m_CI.recompileArguments.includeDirectories)
		command += " -i:" + currentWorkingDir + includeDir;
	if(m_CI.recompileArguments.entryPoint)
		command += " -e:" + std::string(m_CI.recompileArguments.entryPoint);
	if(m_CI.recompileArguments.shaderModel)
		command += " -t:" + std::string(m_CI.recompileArguments.shaderModel);
	for (auto& macro : m_CI.recompileArguments.macros)
		command += " -d:" + std::string(macro);
	if (m_CI.recompileArguments.cso)
		command += " -cso";
	if (m_CI.recompileArguments.spv)
		command += " -spv";
	if (m_CI.recompileArguments.dxcLocation)
		command += " -dxc:" + std::string(m_CI.recompileArguments.dxcLocation);
	if (m_CI.recompileArguments.glslangLocation)
		command += " -glslang:" + std::string(m_CI.recompileArguments.glslangLocation);
	if (m_CI.recompileArguments.additioalArguments)
		command += " -args:" + std::string(m_CI.recompileArguments.additioalArguments);
	if (m_CI.recompileArguments.nologo)
		command += " -nologo";
	if (m_CI.recompileArguments.nooutput)
		command += " -nooutput";

	std::string mscLocation = currentWorkingDir + std::string(m_CI.recompileArguments.mscDirectory);

	MIRU_PRINTF("%s", (std::string("MIRU_CORE: Recompiling shader: ") + currentWorkingDir + m_CI.recompileArguments.hlslFilepath + "\n").c_str());
	MIRU_PRINTF("%s", ("Executing: " + mscLocation + "> " + command + "\n\n").c_str());
	int returnCode = system(("cd " + mscLocation + " && " + command).c_str());
	MIRU_PRINTF("'MIRU_SHADER_COMPILER.exe' has exited with code %d (0x%x).\n", returnCode, returnCode);
	MIRU_PRINTF("%s", "MIRU_CORE: Recompiling shader finished.\n\n");

	if (returnCode != 0)
	{
		MIRU_WARN(returnCode, "WARN: CROSSPLATFORM: MIRU_SHADER_COMIPLER returned an error.");
	}
	else
	{
		Reconstruct();
	}
}