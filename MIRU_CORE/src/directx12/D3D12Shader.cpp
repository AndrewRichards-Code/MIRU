#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Shader.h"

using namespace miru;
using namespace d3d12;

HMODULE Shader::s_HModeuleDXCompiler;
std::filesystem::path Shader::s_DXCompilerFullpath;
uint32_t Shader::s_RefCount = 0;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_ShaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_5;
	MIRU_WARN(m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &m_ShaderModelData, sizeof(m_ShaderModelData)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_MODEL");

	Reconstruct();
	GetShaderResources();
}

Shader::~Shader()
{
	MIRU_CPU_PROFILE_FUNCTION();

	s_RefCount--;
	if (!s_RefCount)
	{
		if (!FreeLibrary(s_HModeuleDXCompiler))
		{
			std::string error_str = "WARN: D3D12: Unable to free '" + s_DXCompilerFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
		}
	}

	m_ShaderByteCode.pShaderBytecode = nullptr;
	m_ShaderByteCode.BytecodeLength = 0;
}

void Shader::Reconstruct()
{
	MIRU_CPU_PROFILE_FUNCTION();

	GetShaderByteCode();
	m_ShaderByteCode.pShaderBytecode = m_ShaderBinary.data();
	m_ShaderByteCode.BytecodeLength = m_ShaderBinary.size();
}

#include "crossplatform/DescriptorPoolSet.h"


void Shader::GetShaderResources()
{
	MIRU_CPU_PROFILE_FUNCTION();

	D3D12ShaderReflection();
}

void Shader::D3D12ShaderReflection()
{
	//Load dxcompiler.dll
	if (!s_HModeuleDXCompiler)
	{
		#if defined(MIRU_WIN64_UWP)
		s_HModeuleDXCompiler = LoadPackagedLibrary(L"dxcompiler.dll", 0);
		#else
		s_DXCompilerFullpath = std::string(PROJECT_DIR) + "redist/dxc/lib/x64/dxcompiler.dll";
		s_HModeuleDXCompiler = LoadLibraryA(s_DXCompilerFullpath.generic_string().c_str());
		#endif
		if (!s_HModeuleDXCompiler)
		{
			std::string error_str = "WARN: D3D12: Unable to load '" + s_DXCompilerFullpath.generic_string() + "'.";
			MIRU_WARN(GetLastError(), error_str.c_str());
			return;
		}
	}
	s_RefCount++;

	IDxcLibrary* dxc_library;
	DxcCreateInstanceProc DxcCreateInstance = (DxcCreateInstanceProc)::GetProcAddress(s_HModeuleDXCompiler, "DxcCreateInstance");
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxc_library));

	IDxcBlobEncoding* dxc_shader_bin;
	dxc_library->CreateBlobWithEncodingFromPinned(m_ShaderByteCode.pShaderBytecode, static_cast<UINT32>(m_ShaderByteCode.BytecodeLength), 0 , &dxc_shader_bin); //binary, no code page

	IDxcContainerReflection* dxc_container_reflection;
	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&dxc_container_reflection));
	dxc_container_reflection->Load(dxc_shader_bin);
	
	uint32_t partIndex;
	uint32_t dxil_kind = 0x4c495844; //MAKEFOURCC('D', 'X', 'I', 'L')
	dxc_container_reflection->FindFirstPartKind(dxil_kind, &partIndex);
	if (partIndex != 4) //No reflection able of shader model 5_1 and lower.
		return;

	//General parsing functions:
	auto D3D_SHADER_INPUT_TYPE_to_miru_crossplatform_DescriptorType = [](D3D_SHADER_INPUT_TYPE type, D3D_SRV_DIMENSION dimension) -> crossplatform::DescriptorType
	{
		switch (type)
		{
		case D3D_SIT_CBUFFER:
			return crossplatform::DescriptorType::UNIFORM_BUFFER;
		case D3D_SIT_TBUFFER:
			return crossplatform::DescriptorType::UNIFORM_TEXEL_BUFFER;
		case D3D_SIT_TEXTURE:
			return crossplatform::DescriptorType::SAMPLED_IMAGE;
		case D3D_SIT_SAMPLER:
			return crossplatform::DescriptorType::SAMPLER;
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		case D3D_SIT_UAV_FEEDBACKTEXTURE:
			return dimension > D3D_SRV_DIMENSION_BUFFER ? crossplatform::DescriptorType::STORAGE_IMAGE : crossplatform::DescriptorType::STORAGE_BUFFER;
		case D3D_SIT_RTACCELERATIONSTRUCTURE:
			return crossplatform::DescriptorType::ACCELERATION_STRUCTURE;
		default:
			break;
		}

		MIRU_ASSERT(true, "ERROR: D3D12: Unsupported D3D_SHADER_INPUT_TYPE and/or D3D_SRV_DIMENSION. Cannot convert to miru::crossplatform::DescriptorType.");
		return static_cast<crossplatform::DescriptorType>(0);
	};
	auto get_shader_stage = [](D3D12_SHADER_VERSION_TYPE type) -> Shader::StageBit
	{
		switch (type)
		{
		case D3D12_SHVER_PIXEL_SHADER:
			return Shader::StageBit::PIXEL_BIT;
		case D3D12_SHVER_VERTEX_SHADER:
			return Shader::StageBit::VERTEX_BIT;
		case D3D12_SHVER_GEOMETRY_SHADER:
			return Shader::StageBit::GEOMETRY_BIT;
		case D3D12_SHVER_HULL_SHADER:
			return Shader::StageBit::HULL_BIT;
		case D3D12_SHVER_DOMAIN_SHADER:
			return Shader::StageBit::DOMAIN_BIT;
		case D3D12_SHVER_COMPUTE_SHADER:
			return Shader::StageBit::COMPUTE_BIT;
		case D3D12_SHVER_RESERVED0:
			return Shader::StageBit(0);
		case 7:
			return Shader::StageBit::RAYGEN_BIT;
		case 8:
			return Shader::StageBit::INTERSECTION_BIT;
		case 9:
			return Shader::StageBit::ANY_HIT_BIT;
		case 10:
			return Shader::StageBit::CLOSEST_HIT_BIT;
		case 11:
			return Shader::StageBit::MISS_BIT;
		case 12:
			return Shader::StageBit::CALLABLE_BIT;
		default:
			return Shader::StageBit(0);
		}
	};
	
	//Shader Reflection
	ID3D12ShaderReflection* shader_reflection = nullptr;
	dxc_container_reflection->GetPartReflection(partIndex, IID_PPV_ARGS(&shader_reflection));
	if (shader_reflection)
	{
		D3D12_SHADER_DESC shaderDesc;
		shader_reflection->GetDesc(&shaderDesc);

		D3D12_SHADER_VERSION_TYPE stage = (D3D12_SHADER_VERSION_TYPE)D3D12_SHVER_GET_TYPE(shaderDesc.Version);
		uint16_t major = D3D12_SHVER_GET_MAJOR(shaderDesc.Version);
		uint16_t minor = D3D12_SHVER_GET_MINOR(shaderDesc.Version);

		D3D_FEATURE_LEVEL featureLevel;
		shader_reflection->GetMinFeatureLevel(&featureLevel);

		auto D3D_REGISTER_COMPONENT_TYPE_to_miru_crossplatform_VertexType = [](D3D_REGISTER_COMPONENT_TYPE type, uint32_t vector_count) -> crossplatform::VertexType
		{
			switch (type)
			{
			case D3D_REGISTER_COMPONENT_UNKNOWN:
				break;
			case D3D_REGISTER_COMPONENT_UINT32:
				return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::UINT)
					+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
			case D3D_REGISTER_COMPONENT_SINT32:
				return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::INT)
					+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
			case D3D_REGISTER_COMPONENT_FLOAT32:
				return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::FLOAT)
					+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
			default:
				break;
			}
			MIRU_ASSERT(true, "ERROR: D3D12: Unsupported D3D_REGISTER_COMPONENT_TYPE. Cannot convert to miru::crossplatform::VertexType.");
			return static_cast<crossplatform::VertexType>(0);
		};

		if (stage == D3D12_SHADER_VERSION_TYPE::D3D12_SHVER_VERTEX_SHADER)
		{
			auto sizeof_miru_crossplatform_VertexType = [](crossplatform::VertexType type) -> uint32_t
			{
				switch (type)
				{
				case miru::crossplatform::VertexType::FLOAT:
				case miru::crossplatform::VertexType::INT:
				case miru::crossplatform::VertexType::UINT:
					return 4;
				case miru::crossplatform::VertexType::VEC2:
				case miru::crossplatform::VertexType::IVEC2:
				case miru::crossplatform::VertexType::UVEC2:
					return 8;
				case miru::crossplatform::VertexType::VEC3:
				case miru::crossplatform::VertexType::IVEC3:
				case miru::crossplatform::VertexType::UVEC3:
					return 12;
				case miru::crossplatform::VertexType::VEC4:
				case miru::crossplatform::VertexType::IVEC4:
				case miru::crossplatform::VertexType::UVEC4:
					return 16;
				case miru::crossplatform::VertexType::DOUBLE:
					return 8;
				case miru::crossplatform::VertexType::DVEC2:
					return 16;
				case miru::crossplatform::VertexType::DVEC3:
					return 24;
				case miru::crossplatform::VertexType::DVEC4:
					return 32;
				default:
					return 0;
				}
			};

			m_VSIADs.clear();
			for (UINT i = 0; i < shaderDesc.InputParameters; i++)
			{
				D3D12_SIGNATURE_PARAMETER_DESC input_parameter;
				shader_reflection->GetInputParameterDesc(i, &input_parameter);

				VertexShaderInputAttributeDescription vsiad;
				vsiad.location = input_parameter.SemanticIndex;
				vsiad.binding = 0;
				vsiad.vertexType = D3D_REGISTER_COMPONENT_TYPE_to_miru_crossplatform_VertexType(input_parameter.ComponentType, (uint32_t)log2((double)(input_parameter.Mask + 1)));
				vsiad.offset = m_VSIADs.empty() ? 0 : m_VSIADs.back().offset + sizeof_miru_crossplatform_VertexType(m_VSIADs.back().vertexType);
				vsiad.semanticName = input_parameter.SemanticName;
				m_VSIADs.push_back(vsiad);
			}
		}

		if (stage == D3D12_SHADER_VERSION_TYPE::D3D12_SHVER_PIXEL_SHADER)
		{
			for (UINT i = 0; i < shaderDesc.OutputParameters; i++)
			{
				m_PSOADs.clear();
				D3D12_SIGNATURE_PARAMETER_DESC out_parameter;
				shader_reflection->GetOutputParameterDesc(i, &out_parameter);

				PixelShaderOutputAttributeDescription psoad;
				psoad.location = out_parameter.SemanticIndex;
				psoad.outputType = D3D_REGISTER_COMPONENT_TYPE_to_miru_crossplatform_VertexType(out_parameter.ComponentType, (uint32_t)log2((double)(out_parameter.Mask + 1)));
				psoad.semanticName = out_parameter.SemanticName;
				m_PSOADs.push_back(psoad);
			}
		}

		for (auto& rbds : m_RBDs)
		{
			rbds.second.clear();
		}
		m_RBDs.clear();

		std::vector<std::string> cis_list;
		for (UINT i = 0; i < shaderDesc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
			shader_reflection->GetResourceBindingDesc(i, &bindingDesc);
			size_t structSize = 0;

			crossplatform::DescriptorType descType = D3D_SHADER_INPUT_TYPE_to_miru_crossplatform_DescriptorType(bindingDesc.Type, bindingDesc.Dimension);
			if (descType == crossplatform::DescriptorType::UNIFORM_BUFFER)
			{
				_D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
				ID3D12ShaderReflectionConstantBuffer* shader_reflection_constant_buffer = shader_reflection->GetConstantBufferByName(bindingDesc.Name);
				shader_reflection_constant_buffer->GetDesc(&shaderBufferDesc);
				structSize = static_cast<size_t>(shaderBufferDesc.Size);
			}

			std::string name = bindingDesc.Name;
			if (name.find("CIS") != std::string::npos)
			{
				bool found = false;
				for (auto& cis : cis_list)
				{
					if (cis.compare(name.substr(0, name.find_last_of('_'))) == 0)
					{
						found = true;
						break;
					}
				}
				if (found)
				{
					continue;
				}
				else
					cis_list.push_back(name.substr(0, name.find_last_of('_')));
				descType = crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER;
			}

			ResourceBindingDescription rbd;
			rbd.binding = bindingDesc.BindPoint;
			rbd.type = descType;
			rbd.descriptorCount = bindingDesc.BindCount;
			rbd.stage = get_shader_stage(stage);
			rbd.name = name;
			rbd.structSize = structSize;
			m_RBDs[bindingDesc.Space][bindingDesc.BindPoint] = rbd;
		}
	}

	//Library Refelection
	ID3D12LibraryReflection* library_reflection = nullptr;
	dxc_container_reflection->GetPartReflection(partIndex, IID_PPV_ARGS(&library_reflection));
	if (library_reflection)
	{
		for (auto& rbds : m_RBDs)
		{
			rbds.second.clear();
		}
		m_RBDs.clear();

		D3D12_LIBRARY_DESC libraryDesc;
		library_reflection->GetDesc(&libraryDesc);
		for(UINT i = 0; i < libraryDesc.FunctionCount; i++)
		{
			ID3D12FunctionReflection* function_reflection = library_reflection->GetFunctionByIndex(i);

			D3D12_FUNCTION_DESC functionDesc;
			function_reflection->GetDesc(&functionDesc);

			D3D12_SHADER_VERSION_TYPE stage = (D3D12_SHADER_VERSION_TYPE)D3D12_SHVER_GET_TYPE(functionDesc.Version);
			uint16_t major = D3D12_SHVER_GET_MAJOR(functionDesc.Version);
			uint16_t minor = D3D12_SHVER_GET_MINOR(functionDesc.Version);

			std::vector<std::string> cis_list;
			for (UINT j = 0; j < functionDesc.BoundResources; j++)
			{
				D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
				function_reflection->GetResourceBindingDesc(j, &bindingDesc);
				size_t structSize = 0;

				crossplatform::DescriptorType descType = D3D_SHADER_INPUT_TYPE_to_miru_crossplatform_DescriptorType(bindingDesc.Type, bindingDesc.Dimension);
				if (descType == crossplatform::DescriptorType::UNIFORM_BUFFER)
				{
					_D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
					ID3D12ShaderReflectionConstantBuffer* shader_reflection_constant_buffer = function_reflection->GetConstantBufferByName(bindingDesc.Name);
					shader_reflection_constant_buffer->GetDesc(&shaderBufferDesc);
					structSize = static_cast<size_t>(shaderBufferDesc.Size);
				}

				std::string name = bindingDesc.Name;
				if (name.find("CIS") != std::string::npos)
				{
					bool found = false;
					for (auto& cis : cis_list)
					{
						if (cis.compare(name.substr(0, name.find_last_of('_'))) == 0)
						{
							found = true;
							break;
						}
					}
					if (found)
					{
						continue;
					}
					else
						cis_list.push_back(name.substr(0, name.find_last_of('_')));
					descType = crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER;
				}

				ResourceBindingDescription& rbd = m_RBDs[bindingDesc.Space][bindingDesc.BindPoint];
				rbd.binding = bindingDesc.BindPoint;
				rbd.type = descType;
				rbd.descriptorCount = bindingDesc.BindCount;
				rbd.stage |= get_shader_stage(stage);
				rbd.name = name;
				rbd.structSize = structSize;
			}
		}
	}

	MIRU_D3D12_SAFE_RELEASE(library_reflection);
	MIRU_D3D12_SAFE_RELEASE(shader_reflection);
	MIRU_D3D12_SAFE_RELEASE(dxc_container_reflection);
	MIRU_D3D12_SAFE_RELEASE(dxc_shader_bin);
	MIRU_D3D12_SAFE_RELEASE(dxc_library);
}
#endif