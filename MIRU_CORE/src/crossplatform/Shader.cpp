#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "directx12/D3D12Shader.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKShader.h"
#endif

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
	shader_core::CompileShaderFromSource(m_CI.recompileArguments);
	Reconstruct();
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
	if (std::filesystem::exists(binFilepath) && std::filesystem::exists(m_CI.recompileArguments.hlslFilepath))
	{
		buildShader = std::filesystem::last_write_time(binFilepath) < std::filesystem::last_write_time(m_CI.recompileArguments.hlslFilepath);
	}

	if (buildShader)
	{
		shader_core::CompileShaderFromSource(m_CI.recompileArguments);
	}
	#endif

	m_ShaderBinary = arc::ReadBinaryFile(binFilepath);
	MIRU_ASSERT(m_ShaderBinary.empty(), "ERROR: CROSSPLATFORM: Unable to read shader binary file.");
}
