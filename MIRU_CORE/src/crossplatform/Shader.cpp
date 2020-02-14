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
	
	filepath = filepath.replace(filepath.find_last_of('.'), 4, shaderBinaryFileExtension);

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

void Shader::Recompile()
{
	std::string binFilepath(m_CI.filepath);
	std::string fileName = binFilepath.substr(binFilepath.find_last_of('/'), binFilepath.find_last_of('.') - binFilepath.find_last_of('/'));

	std::string filepath = "res/shaders" + fileName + ".hlsl";
	std::string outputDir("res/bin");
	std::string command = "MIRU_SHADER_COMPILER -cso -spv -f:" + filepath + " -o:" + outputDir;

	std::string currentWorkingDir = std::filesystem::current_path().string();
	std::string mscLocation;
	#ifdef _DEBUG
	mscLocation = currentWorkingDir + "/../MIRU_SHADER_COMPILER/exe/x64/Debug";
	#else
	mscLocation = currentWorkingDir + "/../MIRU_SHADER_COMPILER/exe/x64/Release";
	#endif

	printf((std::string("MIRU_CORE: Recompiling shader: ") + filepath + "\n").c_str());
	printf(("Executing: " + mscLocation + "> " + command + "\n").c_str());
	int returnCode = system(("cd " + mscLocation + " && " + command).c_str());
	printf("\n");

	MIRU_ASSERT(returnCode, "WARN: CROSSPLATFORM: MIRU_SHADER_COMIPLER returned an error.");

	Reconstruct();
}
