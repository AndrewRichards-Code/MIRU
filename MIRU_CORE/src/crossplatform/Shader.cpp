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
	
	std::string shaderModel = "6_0";
	if (GraphicsAPI::GetAPI() == GraphicsAPI::API::D3D12)
	{
		switch (((d3d12::Shader*)this)->m_ShaderModelData.HighestShaderModel)
		{
		case D3D_SHADER_MODEL_5_1:
			shaderModel = "5_1"; break;
		case D3D_SHADER_MODEL_6_0:
			shaderModel = "6_0"; break;
		case D3D_SHADER_MODEL_6_1:
			shaderModel = "6_1"; break;
		case D3D_SHADER_MODEL_6_2:
			shaderModel = "6_2"; break;
		case D3D_SHADER_MODEL_6_3:
			shaderModel = "6_3"; break;
		case D3D_SHADER_MODEL_6_4:
			shaderModel = "6_4"; break;
		case D3D_SHADER_MODEL_6_5:
			shaderModel = "6_5"; break;
		default:
			shaderModel = "6_0"; break;
		}
	}

	std::string currentWorkingDir = std::filesystem::current_path().string();

	std::string filepath = currentWorkingDir + "/res/shaders" + fileName + ".hlsl";
	std::string outputDir = currentWorkingDir + "/res/bin";
	std::string command = "MIRU_SHADER_COMPILER -cso -spv -f:" + filepath + " -o:" + outputDir + " -t:" + shaderModel;

	std::string mscLocation;
	#ifdef _DEBUG
	mscLocation = currentWorkingDir + "/../MIRU_SHADER_COMPILER/exe/x64/Debug";
	#else
	mscLocation = currentWorkingDir + "/../MIRU_SHADER_COMPILER/exe/x64/Release";
	#endif

	MIRU_PRINTF((std::string("MIRU_CORE: Recompiling shader: ") + filepath + "\n").c_str());
	MIRU_PRINTF(("Executing: " + mscLocation + "> " + command + "\n\n").c_str());
	int returnCode = system(("cd " + mscLocation + " && " + command).c_str());
	MIRU_PRINTF("'MIRU_SHADER_COMPILER.exe' has exited with code %d (0x%x).\n", returnCode, returnCode);
	MIRU_PRINTF("MIRU_CORE: Recompiling shader finished.\n\n");

	MIRU_ASSERT(returnCode, "WARN: CROSSPLATFORM: MIRU_SHADER_COMIPLER returned an error.");
	system("CLS");

	Reconstruct();
}