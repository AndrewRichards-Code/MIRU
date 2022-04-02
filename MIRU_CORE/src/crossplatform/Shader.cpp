#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "directx12/D3D12Shader.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKShader.h"
#endif

#include "ARC/src/FileSaver.h"
#include "ARC/src/FileLoader.h"
#include <dxcapi.h>

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
	CompileShaderFromSource(m_CI.recompileArguments);
	Reconstruct();
	#endif
}

void Shader::CompileShaderFromSource(const CompileArguments& arguments)
{
	MIRU_CPU_PROFILE_FUNCTION();

	std::filesystem::path s_DxilFullpath = GetLibraryFullpath_dxil();
	arc::DynamicLibrary::LibraryHandle s_HModeuleDxil = LoadLibrary_dxil();
	if (!s_HModeuleDxil)
	{
		std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_DxilFullpath.generic_string() + "'.";
		MIRU_WARN(GetLastError(), error_str.c_str());

	}
	std::filesystem::path s_DxcompilerFullpath = GetLibraryFullpath_dxcompiler();
	arc::DynamicLibrary::LibraryHandle s_HModeuleDxcompiler = LoadLibrary_dxcompiler();
	if (!s_HModeuleDxcompiler)
	{
		std::string error_str = "WARN: CROSSPLATFORM: Unable to load '" + s_DxcompilerFullpath.generic_string() + "'.";
		MIRU_WARN(GetLastError(), error_str.c_str());

	}
	DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)arc::DynamicLibrary::LoadFunction(s_HModeuleDxcompiler, "DxcCreateInstance");
	if (!DxcCreateInstance)
		return;

	IDxcCompiler3* compiler = nullptr;
	MIRU_WARN(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)), "WARN: CROSSPLATFORM: DxcCreateInstance failed to create IDxcCompiler3.");

	IDxcUtils* utils = nullptr;
	MIRU_WARN(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)), "WARN: CROSSPLATFORM: DxcCreateInstance failed to create IDxcUtils.");

	if (compiler && utils)
	{
		IDxcIncludeHandler* includeHandler = nullptr;
		MIRU_WARN(utils->CreateDefaultIncludeHandler(&includeHandler), "WARN: CROSSPLATFORM: IDxcUtils::CreateDefaultIncludeHandler failed to create IDxcIncludeHandler.");

		std::string currentWorkingDir = std::filesystem::current_path().string() + "\\";
		IDxcBlobEncoding* sourceBlob = nullptr;
		MIRU_WARN(utils->LoadFile(arc::ToWString(currentWorkingDir + arguments.hlslFilepath).c_str(), nullptr, &sourceBlob), "WARN: CROSSPLATFORM: IDxcUtils::LoadFile failed to create IDxcBlobEncoding.");

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
					c_arguments.push_back(currentWorkingDir + arguments.hlslFilepath);

					size_t fileNamePos = arguments.hlslFilepath.find_last_of('/');
					size_t hlslExtPos = arguments.hlslFilepath.find(".hlsl");
					std::string filename = arguments.hlslFilepath.substr(fileNamePos, hlslExtPos - fileNamePos);
					std::string absoluteDstDir = arguments.outputDirectory + filename + "_" + arguments.shaderModel;
					if (!arguments.entryPoint.empty())
						absoluteDstDir += "_" + arguments.entryPoint;
					if (cso)
						absoluteDstDir += ".cso";
					if (spv)
						absoluteDstDir += ".spv";
					c_arguments.push_back("-Fo");
					c_arguments.push_back(currentWorkingDir + absoluteDstDir);

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
						c_arguments.push_back("-I");
						c_arguments.push_back(currentWorkingDir + includeDir);
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
							std::string absoluteDstPDBDir = absoluteDstDir.replace(absoluteDstDir.find_last_of('.'), 4, ".pdb");
							c_arguments.push_back(currentWorkingDir + absoluteDstPDBDir);
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
						c_arguments.push_back("-fspv-target-env=vulkan1.2");
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
				MIRU_WARN(compiler->Compile(&source, wchar_arguments.data(), static_cast<UINT32>(wchar_arguments.size()), includeHandler, IID_PPV_ARGS(&results)), "WARN: CROSSPLATFORM: IDxcCompiler3::Compile failed.");

				IDxcBlobUtf8* errors = nullptr;
				IDxcBlobUtf16* errorsName = nullptr;
				results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &errorsName);
				if (errors != nullptr && errors->GetStringLength() != 0)
				{
					char* s = (char*)errors->GetStringPointer();
					MIRU_PRINTF("%s", s);
				}

				HRESULT complicationResults = S_OK;
				results->GetStatus(&complicationResults);
				if (complicationResults != S_OK)
				{
					MIRU_WARN(true, "WARN: CROSSPLATFORM: Failed to Compile shader.");
				}

				IDxcBlob* shaderBinary = nullptr;
				IDxcBlobUtf16* shaderName = nullptr;
				results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBinary), &shaderName);
				if (shaderBinary && shaderName)
				{
					char* shaderBinaryPtr = (char*)shaderBinary->GetBufferPointer();
					std::vector<char> contents(shaderBinaryPtr, shaderBinaryPtr + shaderBinary->GetBufferSize());
					arc::SaveBinaryFile(arc::ToString(shaderName->GetStringPointer()), contents);
				}
				else
				{
					MIRU_WARN(true, "WARN: CROSSPLATFORM: Failed to GetOutput for shader binary and/or shader name.");
				}

				if (results->HasOutput(DXC_OUT_PDB))
				{
					IDxcBlob* pdbBinary = nullptr;
					IDxcBlobUtf16* pdbName = nullptr;
					results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBinary), &pdbName);
					if (pdbBinary && pdbName)
					{
						char* pdbBinaryPtr = (char*)pdbBinary->GetBufferPointer();
						std::vector<char> contents(pdbBinaryPtr, pdbBinaryPtr + pdbBinary->GetBufferSize());
						arc::SaveBinaryFile(arc::ToString(pdbName->GetStringPointer()), contents);
					}
					else
					{
						MIRU_WARN(true, "WARN: CROSSPLATFORM: Failed to GetOutput for shader PDB binary and/or shader PDB name.");
					}
				}
			}
		}
		MIRU_D3D12_SAFE_RELEASE(sourceBlob);
		MIRU_D3D12_SAFE_RELEASE(includeHandler);
	}
	MIRU_D3D12_SAFE_RELEASE(utils);
	MIRU_D3D12_SAFE_RELEASE(compiler);

	arc::DynamicLibrary::Unload(s_HModeuleDxcompiler);
	arc::DynamicLibrary::Unload(s_HModeuleDxil);

	return;
}

std::filesystem::path Shader::GetLibraryFullpath_dxil()
{
	return std::string(PROJECT_DIR) + "../External/dxc/bin/x64/dxil.dll";
}

arc::DynamicLibrary::LibraryHandle Shader::LoadLibrary_dxil()
{
	return arc::DynamicLibrary::Load(GetLibraryFullpath_dxil().generic_string());
}

std::filesystem::path Shader::GetLibraryFullpath_dxcompiler()
{
	return std::string(PROJECT_DIR) + "../External/dxc/bin/x64/dxcompiler.dll";
}

arc::DynamicLibrary::LibraryHandle Shader::LoadLibrary_dxcompiler()
{
	return arc::DynamicLibrary::Load(GetLibraryFullpath_dxcompiler().generic_string());
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
		CompileShaderFromSource(m_CI.recompileArguments);
	}
	#endif

	m_ShaderBinary = arc::ReadBinaryFile(binFilepath);
	MIRU_ASSERT(m_ShaderBinary.empty(), "ERROR: CROSSPLATFORM: Unable to read shader binary file.");
}
