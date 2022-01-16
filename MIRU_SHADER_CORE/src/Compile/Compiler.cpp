#include "miru_shader_core_common.h"
#include "Compiler.h"
#include "CompileArguments.h"
#include "Core/DXCLibraryLoader.h"

#include "ARC/src/DynamicLibrary.h"
#include "ARC/src/FileSaver.h"

using namespace miru;
using namespace shader_core;

void miru::shader_core::CompileShaderFromSource(const CompileArguments& arguments)
{
	std::filesystem::path s_DxilFullpath = shader_core::GetLibraryFullpath_dxil();
	arc::DynamicLibrary::LibraryHandle s_HModeuleDxil = shader_core::LoadLibrary_dxil();
	if (!s_HModeuleDxil)
	{
		std::string error_str = "WARN: SHADER_CORE: Unable to load '" + s_DxilFullpath.generic_string() + "'.";
		MIRU_SHADER_CORE_WARN(GetLastError(), error_str.c_str());

	}

	std::filesystem::path s_DxcompilerFullpath = shader_core::GetLibraryFullpath_dxcompiler();
	arc::DynamicLibrary::LibraryHandle s_HModeuleDxcompiler = shader_core::LoadLibrary_dxcompiler();
	if (!s_HModeuleDxcompiler)
	{
		std::string error_str = "WARN: SHADER_CORE: Unable to load '" + s_DxcompilerFullpath.generic_string() + "'.";
		MIRU_SHADER_CORE_WARN(GetLastError(), error_str.c_str());

	}
	DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)arc::DynamicLibrary::LoadFunction(s_HModeuleDxcompiler, "DxcCreateInstance");
	if (!DxcCreateInstance)
		return;

	IDxcCompiler3* compiler = nullptr;
	MIRU_SHADER_CORE_WARN(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)), "WARN: SHADER_CORE: DxcCreateInstance failed to create IDxcCompiler3.");

	IDxcUtils* utils = nullptr;
	MIRU_SHADER_CORE_WARN(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)), "WARN: SHADER_CORE: DxcCreateInstance failed to create IDxcUtils.");

	if (compiler && utils)
	{
		IDxcIncludeHandler* includeHandler = nullptr;
		MIRU_SHADER_CORE_WARN(utils->CreateDefaultIncludeHandler(&includeHandler), "WARN: SHADER_CORE: IDxcUtils::CreateDefaultIncludeHandler failed to create IDxcIncludeHandler.");

		std::string currentWorkingDir = std::filesystem::current_path().string() + "\\";
		IDxcBlobEncoding* sourceBlob = nullptr;
		MIRU_SHADER_CORE_WARN(utils->LoadFile(arc::ToWString(currentWorkingDir + arguments.hlslFilepath).c_str(), nullptr, &sourceBlob), "WARN: SHADER_CORE: IDxcUtils::LoadFile failed to create IDxcBlobEncoding.");

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
				MIRU_SHADER_CORE_WARN(compiler->Compile(&source, wchar_arguments.data(), static_cast<UINT32>(wchar_arguments.size()), includeHandler, IID_PPV_ARGS(&results)), "WARN: SHADER_CORE: IDxcCompiler3::Compile failed.");

				IDxcBlobUtf8* errors = nullptr;
				IDxcBlobUtf16* errorsName = nullptr;
				results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &errorsName);
				if (errors != nullptr && errors->GetStringLength() != 0)
				{
					char* s = (char*)errors->GetStringPointer();
					MIRU_SHADER_CORE_PRINTF("%s", s);
				}

				HRESULT complicationResults = S_OK;
				results->GetStatus(&complicationResults);
				if (complicationResults != S_OK)
				{
					MIRU_SHADER_CORE_WARN(true, "WARN: SHADER_CORE: Failed to Compile shader.");
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
					MIRU_SHADER_CORE_WARN(true, "WARN: SHADER_CORE: Failed to GetOutput for shader binary and/or shader name.");
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
						MIRU_SHADER_CORE_WARN(true, "WARN: SHADER_CORE: Failed to GetOutput for shader PDB binary and/or shader PDB name.");
					}
				}
			}
		}
		MIRU_SHADER_CORE_D3D12_SAFE_RELEASE(sourceBlob);
		MIRU_SHADER_CORE_D3D12_SAFE_RELEASE(includeHandler);
	}
	MIRU_SHADER_CORE_D3D12_SAFE_RELEASE(utils);
	MIRU_SHADER_CORE_D3D12_SAFE_RELEASE(compiler);

	arc::DynamicLibrary::Unload(s_HModeuleDxcompiler);
	arc::DynamicLibrary::Unload(s_HModeuleDxil);

	return;
}
