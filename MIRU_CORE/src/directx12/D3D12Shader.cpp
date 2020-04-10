#include "miru_core_common.h"
#include "D3D12Shader.h"

using namespace miru;
using namespace d3d12;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	m_ShaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_5;
	MIRU_WARN(m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &m_ShaderModelData, sizeof(m_ShaderModelData)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_MODEL");

	GetShaderByteCode();
	m_ShaderByteCode.pShaderBytecode = m_ShaderBinary.data();
	m_ShaderByteCode.BytecodeLength = m_ShaderBinary.size();

	//LoadAssemblyFile();
	//GetShaderResources();
}

Shader::~Shader()
{
	MIRU_CPU_PROFILE_FUNCTION();

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

#include "crossplatform/PipelineHelper.h"
#include "crossplatform/DescriptorPoolSet.h"

void Shader::GetShaderResources()
{
	MIRU_CPU_PROFILE_FUNCTION();

	if (!m_AssemblyFileFound)
		return;
	
	std::string assemblyFileStr(m_ShaderAssembly.data());
	std::stringstream assemblyFileSS(assemblyFileStr);

	std::vector<std::string> items;
	if (m_CI.stage == Shader::StageBit::VERTEX_BIT)
	{
		if (assemblyFileStr.find("Vertex Shader", 0) == std::string::npos)
			MIRU_ASSERT(true, "ERROR: D3D12: The provided assembly file doesn't match the specified stage: StageBit::VERTEX_BIT.");
		
		std::string line;
		bool inputSigFound = false;
		bool inputSigTypeInfoFound = false;
		while (!assemblyFileSS.eof())
		{
			std::getline(assemblyFileSS, line);
			if (line.compare("; Input signature:") == 0)
			{
				inputSigFound = true;
				continue;
			}
			if (line.compare("; Name                 Index   Mask Register SysValue  Format   Used") == 0 && inputSigFound)
			{
				inputSigTypeInfoFound = true;
				continue;
			}
			if (inputSigFound && inputSigTypeInfoFound)
			{
				if (line.compare(";") == 0) //End of Input signature
					break;

				std::string word;
				std::stringstream _tmp_ss(line);
				while (!_tmp_ss.eof())
				{
					_tmp_ss >> word;
					if (word.find(';', 0) != std::string::npos)
						continue;
					else if (word.find('-', 0) != std::string::npos)
						break;
					else
					{
						items.push_back(word);
					}
				}
				if(!items.empty())
				{
					m_VSIADs.push_back({});
					m_VSIADs.back().location = atoi(items[3].c_str());
					m_VSIADs.back().binding = 0;
					m_VSIADs.back().vertexType = (crossplatform::VertexType)((items[2].size() - 1) + (
						(items[5].compare("float") == 0) ? 0 :
						(items[5].compare("double") == 0) ? 4 :
						(items[5].compare("int") == 0) ? 8 :
						(items[5].compare("uint") ==0)  ? 12 : 0));
					m_VSIADs.back().offset = m_VSIADs.size() == 1 ? 0 : (((uint32_t)m_VSIADs[m_VSIADs.size() - 2].vertexType % (uint32_t)4)+(uint32_t)1) 
						* (m_VSIADs[m_VSIADs.size() - 2].vertexType >= crossplatform::VertexType::DOUBLE ? (uint32_t)8 : (uint32_t)4) + m_VSIADs[m_VSIADs.size() - 2].offset;
					m_VSIADs.back().semanticName = items[0];
					items.clear();
				}
				continue;
			}
		}
	}
	if (m_CI.stage == Shader::StageBit::PIXEL_BIT)
	{
		if (assemblyFileStr.find("Pixel Shader", 0) == std::string::npos)
			MIRU_ASSERT(true, "ERROR: D3D12: The provided assembly file doesn't match the specified stage: StageBit::PIXEL_BIT.");
		
		std::string line;
		bool outputSigFound = false;
		bool outputSigTypeInfoFound = false;
		while (!assemblyFileSS.eof())
		{
			std::getline(assemblyFileSS, line);
			if (line.compare("; Output signature:") == 0)
			{
				outputSigFound = true;
				continue;
			}
			if (line.compare("; Name                 Index   Mask Register SysValue  Format   Used") == 0 && outputSigFound)
			{
				outputSigTypeInfoFound = true;
				continue;
			}
			if (outputSigFound && outputSigTypeInfoFound)
			{
				if (line.compare(";") == 0) //End of Output signature
					break;

				std::string word;
				std::stringstream _tmp_ss(line);
				while (!_tmp_ss.eof())
				{
					_tmp_ss >> word;
					if (word.find(';', 0) != std::string::npos)
						continue;
					else if (word.find('-', 0) != std::string::npos)
						break;
					else
					{
						items.push_back(word);
					}
				}
				if (!items.empty())
				{
					m_PSOADs.push_back({});
					m_PSOADs.back().binding = atoi(items[3].c_str());;
					m_PSOADs.back().outputType = (crossplatform::VertexType)((items[2].size() - 1) + (
						(items[5].compare("float") == 0) ? 0 :
						(items[5].compare("double") == 0) ? 4 :
						(items[5].compare("int") == 0) ? 8 :
						(items[5].compare("uint") == 0) ? 12 : 0));
					m_PSOADs.back().semanticName = items[0];
					items.clear();
				}
				continue;
			}
		}
	}
	//ShaderResourceBindingDescription
	{
		std::vector<std::string> cis_list;

		std::string line;
		bool resDefFound = false;
		bool resDefInfoFound = false;
		while (!assemblyFileSS.eof())
		{
			std::getline(assemblyFileSS, line);
			if (line.compare("; Resource Bindings:") == 0)
			{
				resDefFound = true;
				continue;
			}
			if (line.compare("; Name                                 Type  Format         Dim      ID      HLSL Bind  Count") == 0 && resDefFound)
			{
				resDefInfoFound = true;
				continue;
			}
			if (resDefFound && resDefInfoFound)
			{
				if (line.compare(";") == 0) //End of Output signature
					break;

				std::string word;
				std::stringstream _tmp_ss(line);
				while (!_tmp_ss.eof())
				{
					_tmp_ss >> word;
					if (word.find(';', 0) != std::string::npos)
						continue;
					else if (word.find('-', 0) != std::string::npos)
						break;
					else
					{
						items.push_back(word);
					}
				}
				if (!items.empty())
				{
					uint32_t set = 0;
					if (items[5].find("space") != std::string::npos)
						set = atoi(items[5].c_str() + items[5].size() - 1);

					std::string bindingStr;
					for (auto& _char : items[5])
					{
						if (_char == '0' || _char == '1' || _char == '2' || _char == '3' || _char == '4' || _char == '5' || _char == '6' || _char == '7' || _char == '8' || _char == '9')
							bindingStr += _char;
						if (_char == ',')
							break;
					}
					uint32_t binding = 0;
					if (!bindingStr.empty())
						binding = atoi(bindingStr.c_str());

					crossplatform::DescriptorType type;
					if (items[0].find("_cis") != std::string::npos)
					{
						bool found = false;
						for (auto& cis : cis_list)
						{
							if (cis.compare(items[0].substr(0, items[0].find_first_of('_'))) == 0)
							{
								found = true;
								break;
							}
						}
						if (found)
						{
							items.clear();
							continue;
						}
						else
							cis_list.push_back(items[0].substr(0, items[0].find_first_of('_')));
							type = crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER;
					}
					else
					{
						if (items[1].compare("cbuffer") == 0)
							type = crossplatform::DescriptorType::UNIFORM_BUFFER;
						else if (items[1].compare("sampler") == 0)
							type = crossplatform::DescriptorType::SAMPLER;
						else if (items[1].compare("texture") == 0)
							type = crossplatform::DescriptorType::SAMPLED_IMAGE;
						else
							MIRU_ASSERT(true, "ERROR: D3D12: Unknown Resource Binding Type.");
					}

					m_RBDs[set].push_back({});
					m_RBDs[set].back().binding = binding;
					m_RBDs[set].back().type = type;
					m_RBDs[set].back().descriptorCount = atoi(items[6].c_str());
					m_RBDs[set].back().stage = m_CI.stage;
					items.clear();
				}
				continue;
			}
		}
	}
}
