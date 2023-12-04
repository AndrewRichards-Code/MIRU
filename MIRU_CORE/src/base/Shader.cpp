#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Shader.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKShader.h"
#endif

#include "ARC/src/FileSaver.h"
#include "ARC/src/FileLoader.h"
#include "ARC/External/JSON/json.hpp"

#if defined(_WIN64)
#include "build/native/include/dxcapi.h"
#endif

#include <fstream>
#include <regex>

using namespace miru;
using namespace base;

Shader::~Shader()
{
}

ShaderRef Shader::Create(Shader::CreateInfo* pCreateInfo)
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
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

void Shader::Recompile()
{
	MIRU_CPU_PROFILE_FUNCTION();

	#if !defined(MIRU_WIN64_UWP)
	CompileShaderFromSource(m_CI.recompileArguments);
	Reconstruct();
	#endif
}

std::vector<Shader::CompileArguments> Shader::LoadCompileArgumentsFromFile(std::filesystem::path filepath, const std::unordered_map<std::string, std::string>& environmentVariables)
{
	std::vector<Shader::CompileArguments> compileArguments = {};

	std::filesystem::path currentWorkingDir = std::filesystem::current_path();
	if (filepath.is_relative())
		filepath = currentWorkingDir / filepath.relative_path();

	using namespace nlohmann;
	json jsonData = {};
	std::ifstream file(filepath, std::ios::binary);
	if (file.is_open())
	{
		file >> jsonData;
	}
	else
	{
		MIRU_WARN(true, "WARN: BASE: The Recompile Arguments File could not by opened.");
		return compileArguments;
	}
	file.close();

	if (jsonData["fileType"] != "MSC_RAF")
	{
		MIRU_WARN(true, "WARN: BASE: The Recompile Arguments File is not valid.");
		return compileArguments;
	}

	const json& recompileArguments = jsonData["recompileArguments"];
	for (const json& recompileArgument : recompileArguments)
	{
		CompileArguments compileArgument;
		compileArgument.hlslFilepath = recompileArgument["hlslFilepath"];
		compileArgument.outputDirectory = recompileArgument["outputDirectory"];
		compileArgument.includeDirectories = { recompileArgument["includeDirectories"].begin(), recompileArgument["includeDirectories"].end() };
		compileArgument.entryPoint = recompileArgument["entryPoint"];
		compileArgument.shaderModel = recompileArgument["shaderModel"];
		compileArgument.macros = { recompileArgument["macros"].begin(), recompileArgument["macros"].end() };
		compileArgument.cso = recompileArgument["cso"];
		compileArgument.spv = recompileArgument["spv"];
		compileArgument.dxcArguments = { recompileArgument["dxcArguments"].begin(), recompileArgument["dxcArguments"].end() };

		for (const auto& environmentVariable : environmentVariables)
		{
			const std::string& regexVariable = "\\" + environmentVariable.first;
			const std::string& value = environmentVariable.second;

			compileArgument.hlslFilepath = std::regex_replace(compileArgument.hlslFilepath, std::regex(regexVariable), value);
			compileArgument.outputDirectory = std::regex_replace(compileArgument.outputDirectory, std::regex(regexVariable), value);
			for (auto& includeDirectory : compileArgument.includeDirectories)
				includeDirectory = std::regex_replace(includeDirectory, std::regex(regexVariable), value);
		}

		compileArguments.push_back(compileArgument);
	}

	return compileArguments;
}

void Shader::CompileShaderFromSource(const CompileArguments& arguments)
{
	MIRU_CPU_PROFILE_FUNCTION();

	#if defined(_WIN64)
	IDxcCompiler3* compiler = nullptr;
	MIRU_WARN(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)), "WARN: BASE: DxcCreateInstance failed to create IDxcCompiler3.");

	IDxcUtils* utils = nullptr;
	MIRU_WARN(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)), "WARN: BASE: DxcCreateInstance failed to create IDxcUtils.");

	if (compiler && utils)
	{
		IDxcIncludeHandler* includeHandler = nullptr;
		MIRU_WARN(utils->CreateDefaultIncludeHandler(&includeHandler), "WARN: BASE: IDxcUtils::CreateDefaultIncludeHandler failed to create IDxcIncludeHandler.");

		std::filesystem::path currentWorkingDir = std::filesystem::current_path();

		std::filesystem::path absoluteHlslFilepath = std::filesystem::path(arguments.hlslFilepath);
		if (absoluteHlslFilepath.is_relative())
			absoluteHlslFilepath = currentWorkingDir / absoluteHlslFilepath.relative_path();

		IDxcBlobEncoding* sourceBlob = nullptr;
		MIRU_WARN(utils->LoadFile(arc::ToWString(absoluteHlslFilepath.string()).c_str(), nullptr, &sourceBlob), "WARN: BASE: IDxcUtils::LoadFile failed to create IDxcBlobEncoding.");

		if (includeHandler && sourceBlob)
		{
			DxcBuffer source;
			source.Ptr = sourceBlob->GetBufferPointer();
			source.Size = sourceBlob->GetBufferSize();
			source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

			uint32_t count = (arguments.cso ? 1 : 0) + (arguments.spv ? 1 : 0);
			for (uint32_t i = 0; i < count; i++)
			{
				bool cso = false;
				bool spv = false;
				if (count == 2)
				{
					cso = (i == 0);
					spv = (i == 1);
				}
				else
				{
					cso = arguments.cso;
					spv = arguments.spv;
				}

				std::vector<std::string> c_arguments;
				{
					c_arguments.push_back(absoluteHlslFilepath.string());

					std::string filename = absoluteHlslFilepath.stem().string();
					std::string dstFilename = filename + "_" + arguments.shaderModel;
					if (!arguments.entryPoint.empty())
						dstFilename += "_" + arguments.entryPoint;
					if (cso)
						dstFilename += ".cso";
					if (spv)
						dstFilename += ".spv";

					std::filesystem::path absoluteDstDir = arguments.outputDirectory / std::filesystem::path(dstFilename);
					if (absoluteDstDir.is_relative())
						absoluteDstDir = currentWorkingDir / absoluteDstDir.relative_path();
					c_arguments.push_back("-Fo");
					c_arguments.push_back(absoluteDstDir.string());

					if (!arguments.entryPoint.empty())
					{
						c_arguments.push_back("-E");
						c_arguments.push_back(arguments.entryPoint);
					}
					if (!arguments.shaderModel.empty())
					{
						c_arguments.push_back("-T");
						c_arguments.push_back(arguments.shaderModel);
					}
					for (auto& includeDir : arguments.includeDirectories)
					{
						std::filesystem::path absoluteIncludeDir = includeDir;
						if (absoluteIncludeDir.is_relative())
							absoluteIncludeDir = currentWorkingDir / absoluteIncludeDir.relative_path();

						c_arguments.push_back("-I");
						c_arguments.push_back(absoluteIncludeDir.string());
					}
					for (auto& macro : arguments.macros)
					{
						c_arguments.push_back("-D");
						c_arguments.push_back(macro);
					}
					for (auto& arg : arguments.dxcArguments)
					{
						c_arguments.push_back(arg);

						if (arg.find("-Fd") != std::string::npos)
						{
							if (spv)
							{
								c_arguments.pop_back();
								continue;
							}
							std::filesystem::path absoluteDstPDBDir = absoluteDstDir.replace_extension(std::filesystem::path(".pdb"));
							c_arguments.push_back(absoluteDstPDBDir.string());
						}
						if (arg.find("-Zi") != std::string::npos && spv)
						{
							c_arguments.push_back("-fspv-debug=line");
						}
					}
					if (cso)
					{
						c_arguments.push_back("-D");
						c_arguments.push_back("MIRU_D3D12");
					}
					if (spv)
					{
						c_arguments.push_back("-D");
						c_arguments.push_back("MIRU_VULKAN");
						if (arguments.shaderModel.find("ps") != std::string::npos)
						{
							c_arguments.push_back("-D");
							c_arguments.push_back("MIRU_FRAGMENT_SHADER");
						}
						c_arguments.push_back("-spirv");
						c_arguments.push_back("-fspv-target-env=vulkan1.3");
					}
				}
				std::vector<std::wstring> w_arguments;
				w_arguments.reserve(c_arguments.size());
				for (const auto& arg : c_arguments)
					w_arguments.push_back(arc::ToWString(arg));

				std::vector<LPCWSTR> wchar_arguments;
				wchar_arguments.reserve(c_arguments.size());
				for (const auto& arg : w_arguments)
					wchar_arguments.push_back(arg.c_str());

				IDxcResult* results = nullptr;
				MIRU_WARN(compiler->Compile(&source, wchar_arguments.data(), static_cast<UINT32>(wchar_arguments.size()), includeHandler, IID_PPV_ARGS(&results)), "WARN: BASE: IDxcCompiler3::Compile failed.");

				auto LogDXCCommandLineArgs = [&]()
					{
						std::stringstream ss;
						for (const auto& arg : c_arguments)
						{
							ss << arg << " ";
						}
						MIRU_WARN(true, ss.str().c_str());
					};

				IDxcBlobUtf8* errors = nullptr;
				IDxcBlobUtf16* errorsName = nullptr;
				results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &errorsName);
				if (errors != nullptr && errors->GetStringLength() != 0)
				{
					LogDXCCommandLineArgs();
					MIRU_WARN(true, (char*)errors->GetStringPointer());
				}
				MIRU_D3D12_SAFE_RELEASE(errorsName);
				MIRU_D3D12_SAFE_RELEASE(errors);

				HRESULT complicationResults = S_OK;
				results->GetStatus(&complicationResults);
				if (complicationResults != S_OK)
				{
					LogDXCCommandLineArgs();
					MIRU_WARN(true, "WARN: BASE: Failed to Compile shader.");
				}

				IDxcBlob* shaderBinary = nullptr;
				IDxcBlobUtf16* shaderName = nullptr;
				results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBinary), &shaderName);
				if (shaderBinary && shaderName)
				{
					char* shaderBinaryPtr = (char*)shaderBinary->GetBufferPointer();
					std::vector<char> contents(shaderBinaryPtr, shaderBinaryPtr + shaderBinary->GetBufferSize());
					std::filesystem::path shaderBinaryPath = arc::ToString(shaderName->GetStringPointer());
					std::filesystem::create_directory(shaderBinaryPath.parent_path());
					arc::SaveBinaryFile(shaderBinaryPath.string(), contents);
				}
				else
				{
					LogDXCCommandLineArgs();
					MIRU_WARN(true, "WARN: BASE: Failed to GetOutput for shader binary and/or shader name.");
				}
				MIRU_D3D12_SAFE_RELEASE(shaderName);
				MIRU_D3D12_SAFE_RELEASE(shaderBinary);

				if (results->HasOutput(DXC_OUT_PDB))
				{
					IDxcBlob* pdbBinary = nullptr;
					IDxcBlobUtf16* pdbName = nullptr;
					results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBinary), &pdbName);
					if (pdbBinary && pdbName)
					{
						char* pdbBinaryPtr = (char*)pdbBinary->GetBufferPointer();
						std::vector<char> contents(pdbBinaryPtr, pdbBinaryPtr + pdbBinary->GetBufferSize());
						std::filesystem::path pdbBinaryPath = arc::ToString(pdbName->GetStringPointer());
						std::filesystem::create_directory(pdbBinaryPath.parent_path());
						arc::SaveBinaryFile(pdbBinaryPath.string(), contents);
					}
					else
					{
						LogDXCCommandLineArgs();
						MIRU_WARN(true, "WARN: BASE: Failed to GetOutput for shader PDB binary and/or shader PDB name.");
					}
					MIRU_D3D12_SAFE_RELEASE(pdbName);
					MIRU_D3D12_SAFE_RELEASE(pdbBinary);
				}
				
				MIRU_D3D12_SAFE_RELEASE(results);
			}
		}
		MIRU_D3D12_SAFE_RELEASE(sourceBlob);
		MIRU_D3D12_SAFE_RELEASE(includeHandler);
	}
	MIRU_D3D12_SAFE_RELEASE(utils);
	MIRU_D3D12_SAFE_RELEASE(compiler);
	#endif

	return;
}

void Shader::GetShaderByteCode()
{
	MIRU_CPU_PROFILE_FUNCTION();

	//Check for valid shader source
	std::string binFilepath;
	if(!m_CI.binaryFilepath.empty())
		binFilepath = std::string(m_CI.binaryFilepath);

	if (binFilepath.empty() && m_CI.binaryCode.empty())
		MIRU_FATAL(true, "ERROR: BASE: No file path or binary code provided.");

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
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return;
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
		CompileShaderFromSource(m_CI.recompileArguments);
	}
	#endif

	m_ShaderBinary = arc::ReadBinaryFile(binFilepath);
	MIRU_FATAL(m_ShaderBinary.empty(), "ERROR: BASE: Unable to read shader binary file.");
}
