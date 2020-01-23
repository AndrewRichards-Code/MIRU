#include "common.h"
#include "directx12/D3D12Shader.h"
#include "vulkan/VKShader.h"

using namespace miru;
using namespace crossplatform;

Ref<Shader> Shader::Create(Shader::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		return std::make_shared<d3d12::Shader>(pCreateInfo);
	case GraphicsAPI::API::VULKAN:
		return std::make_shared<vulkan::Shader>(pCreateInfo);
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unknown GraphicsAPI."); return nullptr;
	}
}

void Shader::GetShaderByteCode()
{
	std::string filepath(m_CI.filepath);
	if (filepath.empty())
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: No file path provided.");

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
	
	m_CI.filepath = filepath.replace(filepath.find_last_of('.'), 4, shaderBinaryFileExtension).c_str();

	std::ifstream stream(filepath, std::ios::ate | std::ios::binary);
	if (!stream.is_open())
	{
		MIRU_ASSERT(true, "ERROR: CROSSPLATFORM: Unable to read shader binary file.");
	}
	m_ShaderBinary.resize(static_cast<size_t>(stream.tellg()));
	stream.seekg(0);
	stream.read(m_ShaderBinary.data(), static_cast<std::streamsize>(m_ShaderBinary.size()));
	stream.close();
}
