#include "common.h"
#include "VKShader.h"

using namespace miru;
using namespace vulkan;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;
	GetShaderByteCode();

	m_ShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	m_ShaderModuleCI.pNext = nullptr;
	m_ShaderModuleCI.flags = 0;
	m_ShaderModuleCI.codeSize = static_cast<uint32_t>(m_ShaderBinary.size());
	m_ShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(m_ShaderBinary.data());

	MIRU_ASSERT(vkCreateShaderModule(m_Device, &m_ShaderModuleCI, nullptr, &m_ShaderModule), "ERROR: VULKAN: Failed to create ShaderModule.");
	VKSetName<VkShaderModule>(m_Device, (uint64_t)m_ShaderModule, m_CI.debugName);
	
	m_ShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_ShaderStageCI.pNext = nullptr;
	m_ShaderStageCI.flags = 0;
	m_ShaderStageCI.stage = static_cast<VkShaderStageFlagBits>(m_CI.stage);
	m_ShaderStageCI.module = m_ShaderModule;
	m_ShaderStageCI.pName = m_CI.entryPoint;
	m_ShaderStageCI.pSpecializationInfo = nullptr;

	GetShaderResources();
}

Shader::~Shader()
{
	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
}

void Shader::Reconstruct()
{
	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);

	GetShaderByteCode();

	m_ShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	m_ShaderModuleCI.pNext = nullptr;
	m_ShaderModuleCI.flags = 0;
	m_ShaderModuleCI.codeSize = static_cast<uint32_t>(m_ShaderBinary.size());
	m_ShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(m_ShaderBinary.data());

	MIRU_ASSERT(vkCreateShaderModule(m_Device, &m_ShaderModuleCI, nullptr, &m_ShaderModule), "ERROR: VULKAN: Failed to create ShaderModule.");
	VKSetName<VkShaderModule>(m_Device, (uint64_t)m_ShaderModule, m_CI.debugName);

	m_ShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_ShaderStageCI.pNext = nullptr;
	m_ShaderStageCI.flags = 0;
	m_ShaderStageCI.stage = static_cast<VkShaderStageFlagBits>(m_CI.stage);
	m_ShaderStageCI.module = m_ShaderModule;
	m_ShaderStageCI.pName = m_CI.entryPoint;
	m_ShaderStageCI.pSpecializationInfo = nullptr;
}

#include "vulkan/spirv.h"
#include "crossplatform/DescriptorPoolSet.h"

struct MiruSpvId
{
	SpvOp opcode = SpvOp::SpvOpMax;
	uint32_t referenceId = ~0;
	SpvStorageClass storageClass = SpvStorageClass::SpvStorageClassMax;
	uint32_t set = ~0;
	uint32_t binding = ~0;
	uint32_t count = ~0;
	uint32_t location = ~0;
	bool signedType = false;
	std::string name;
};

void Shader::GetShaderResources()
{
	const uint32_t* code = m_ShaderModuleCI.pCode;
	if (code[0] != SpvMagicNumber)
		MIRU_ASSERT(true, "ERROR: VULKAN: Provided SPIR-V source file is not valid.");

	std::vector<MiruSpvId> ids(code[3]);
	SpvExecutionModel stage;

	const uint32_t* _code = code + uint32_t(5);
	while (_code < code + m_ShaderModuleCI.codeSize)
	{
		uint16_t opcode = uint16_t(_code[0] & SpvOpCodeMask);
		uint16_t wordCount = uint16_t(_code[0] >> SpvWordCountShift);

		switch (opcode)
		{
		case SpvOpEntryPoint:
		{
			stage = SpvExecutionModel(_code[1]);
			break;
		}
		case SpvOpName:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp(opcode);
			ids[id].name = (const char*)(&_code[2]);
			break;
		}
		case SpvOpDecorate:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp(opcode);
			SpvDecoration dec = SpvDecoration(_code[2]);
			switch (dec)
			{
			case SpvDecorationLocation:
			{
				ids[id].location = _code[3];
				break;
			}
			case SpvDecorationDescriptorSet:
			{
				ids[id].set = _code[3];
				break;
			}
			case SpvDecorationBinding:
			{
				ids[id].binding = _code[3];
				break;
			}
			}
			break;
		}
		case SpvOpTypeInt:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp_(opcode);
			ids[id].count = _code[2];
			ids[id].signedType= (bool)_code[3];
			break;
		}
		case SpvOpTypeFloat:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp_(opcode);
			ids[id].count = _code[2];
			break;
		}
		case SpvOpTypeVector:
		case SpvOpTypeMatrix:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp_(opcode);
			ids[id].referenceId = _code[2];
			ids[id].count = _code[3];
			break;
		}
		case SpvOpTypeVoid:
		case SpvOpTypeBool:
		case SpvOpTypeStruct:
		case SpvOpTypeImage:
		case SpvOpTypeSampler:
		case SpvOpTypeSampledImage:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp_(opcode);
			break;
		}
		case SpvOpTypePointer:
		{
			uint32_t id = _code[1];
			ids[id].opcode = SpvOp_(opcode);
			ids[id].storageClass = SpvStorageClass(_code[2]);
			ids[id].referenceId = _code[3];
			break;
		}
		case SpvOpVariable:
		{
			uint32_t id = _code[2];
			ids[id].opcode = SpvOp_(opcode);
			ids[id].referenceId = _code[1];
			ids[id].storageClass = SpvStorageClass(_code[3]);
			break;
		}

		}

		_code += size_t(wordCount);
	}
	if ((uint32_t)stage != (uint32_t)log2((double)m_CI.stage)) // Convert Bitfield to uint32_t
		MIRU_ASSERT(true, "ERROR: VULKAN: The SPIR-V source file doesn't match the specified stage.");

	std::vector<std::string> cis_list;
	for (auto id : ids)
	{
		if (id.opcode != SpvOpVariable)
			continue;
		if ((uint32_t)stage == (uint32_t)SpvExecutionModel::SpvExecutionModelVertex
			&& (uint32_t)id.storageClass == (uint32_t)SpvStorageClassInput)
		{
			MiruSpvId& typeID = ids[ids[id.referenceId].referenceId];
			crossplatform::VertexType vertexType;
			uint32_t count, enumOffset;
			if (typeID.referenceId != ~0) // Either Vector or Matrix type.
			{
				count = typeID.count;
				switch (ids[typeID.referenceId].opcode)
				{
					case SpvOpTypeInt:
						enumOffset = (ids[typeID.referenceId].signedType ? 8 : 12); break;
					case SpvOpTypeFloat:
						enumOffset = (ids[typeID.referenceId].count == 32 ? 0 : 4); break;
				}
			}
			else //Either Int of Float base type 
			{
				count = 1;
				switch (typeID.opcode)
				{
				case SpvOpTypeInt:
					enumOffset = (typeID.signedType ? 8 : 12); break;
				case SpvOpTypeFloat:
					enumOffset = (typeID.count == 32 ? 0 : 4); break;
				}
			}
			vertexType = (crossplatform::VertexType)((count - 1) + enumOffset);
			uint32_t offest = m_VSIADs.size() == 0 ? 0 : (((uint32_t)m_VSIADs[m_VSIADs.size() - 1].vertexType % (uint32_t)4)+(uint32_t)1) 
				* (m_VSIADs[m_VSIADs.size() - 1].vertexType >= crossplatform::VertexType::DOUBLE ? (uint32_t)8 : (uint32_t)4) + m_VSIADs[m_VSIADs.size() - 1].offset;
			m_VSIADs.push_back({id.location, 0, vertexType, offest, id.name});
			continue;
		}
		if ((uint32_t)stage == (uint32_t)SpvExecutionModel::SpvExecutionModelFragment
			&& (uint32_t)id.storageClass == (uint32_t)SpvStorageClassOutput)
		{
			MiruSpvId& typeID = ids[ids[id.referenceId].referenceId];
			crossplatform::VertexType type;
			uint32_t count, enumOffset;
			if (typeID.referenceId != ~0) // Either Vector or Matrix type.
			{
				count = typeID.count;
				switch (ids[typeID.referenceId].opcode)
				{
				case SpvOpTypeInt:
					enumOffset = (ids[typeID.referenceId].signedType ? 8 : 12); break;
				case SpvOpTypeFloat:
					enumOffset = (ids[typeID.referenceId].count == 32 ? 0 : 4); break;
				}
			}
			else //Either Int of Float base type 
			{
				count = 1;
				switch (typeID.opcode)
				{
				case SpvOpTypeInt:
					enumOffset = (typeID.signedType ? 8 : 12); break;
				case SpvOpTypeFloat:
					enumOffset = (typeID.count == 32 ? 0 : 4); break;
				}
			}
			type = (crossplatform::VertexType)((count - 1) + enumOffset);
			m_PSOADs.push_back({ id.location, type, id.name });
			continue;
		}
		//ShaderResourceBindingDescription
		if ((uint32_t)id.storageClass != (uint32_t)SpvStorageClassInput
			&& (uint32_t)id.storageClass != (uint32_t)SpvStorageClassOutput)
		{
			uint32_t& refId = id.referenceId;
			MiruSpvId& ref = ids[refId];

			uint32_t& refTypeId = ref.referenceId;
			MiruSpvId& refType = ids[refTypeId];
			crossplatform::DescriptorType type = crossplatform::DescriptorType(0);
			if (id.name.find("_cis") != std::string::npos)
			{
				bool found = false;
				for (auto& cis : cis_list)
				{
					if (cis.compare(id.name.substr(0, id.name.find_first_of('_'))) == 0)
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
					cis_list.push_back(id.name.substr(0, id.name.find_first_of('_')));
					type = crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER;
			}
			else
			{
				switch (refType.opcode)
				{
				case SpvOpTypeStruct:
				{
					if (ref.storageClass == SpvStorageClassUniform)
						type = crossplatform::DescriptorType::UNIFORM_BUFFER; break;
					if (ref.storageClass == SpvStorageClassStorageBuffer)
						type = crossplatform::DescriptorType::STORAGE_BUFFER; break;

				}
				case SpvOpTypeImage:
					type = crossplatform::DescriptorType::STORAGE_IMAGE; break;
				case SpvOpTypeSampler:
					type = crossplatform::DescriptorType::SAMPLER; break;
				case SpvOpTypeSampledImage:
					type = crossplatform::DescriptorType::SAMPLED_IMAGE; break;
				};
			}
			m_RBD[id.set].push_back({ id.binding, type, 1, m_CI.stage });
		}
	}
}