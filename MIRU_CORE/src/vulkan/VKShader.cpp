#include "miru_core_common.h"
#include "VKShader.h"

using namespace miru;
using namespace vulkan;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

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
	m_ShaderStageCI.pName = m_CI.entryPoint.c_str();
	m_ShaderStageCI.pSpecializationInfo = nullptr;
	
	GetShaderResources();
}

Shader::~Shader()
{
	MIRU_CPU_PROFILE_FUNCTION();

	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
}

void Shader::Reconstruct()
{
	MIRU_CPU_PROFILE_FUNCTION();

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
	m_ShaderStageCI.pName = m_CI.entryPoint.c_str();
	m_ShaderStageCI.pSpecializationInfo = nullptr;
}

#include "crossplatform/DescriptorPoolSet.h"

void Shader::GetShaderResources()
{
	MIRU_CPU_PROFILE_FUNCTION();

	SpirvCrossReflection();
}

void Shader::SpirvCrossReflection()
{
	const uint32_t* spv_bin = reinterpret_cast<const uint32_t*>(m_ShaderBinary.data());
	size_t spv_bin_word_count = m_ShaderBinary.size() / 4;
	spirv_cross::Compiler compiled_bin(spv_bin, spv_bin_word_count);
	spirv_cross::ShaderResources resources = compiled_bin.get_shader_resources();

	spv::ExecutionModel stage = compiled_bin.get_execution_model();
	if ((uint32_t)stage != (uint32_t)log2((double)m_CI.stage)) // Convert Bitfield to uint32_t
		MIRU_ASSERT(true, "ERROR: VULKAN: The SPIR-V source file doesn't match the specified stage.");

	auto spirv_cross_SPIRType_BaseType_to_miru_crossplatform_VertexType = 
		[](spirv_cross::SPIRType::BaseType type, uint32_t vector_count) -> crossplatform::VertexType
	{
		switch (type)
		{
		case spirv_cross::SPIRType::BaseType::Unknown:
			break;
		case spirv_cross::SPIRType::BaseType::Void:
			break;
		case spirv_cross::SPIRType::BaseType::Boolean:
			break;
		case spirv_cross::SPIRType::BaseType::SByte:
			break;
		case spirv_cross::SPIRType::BaseType::UByte:
			break;
		case spirv_cross::SPIRType::BaseType::Short:
			break;
		case spirv_cross::SPIRType::BaseType::UShort:
			break;
		case spirv_cross::SPIRType::BaseType::Int:
			return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::INT) 
				+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::UInt:
			return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::UINT) 
				+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::Int64:
			break;
		case spirv_cross::SPIRType::BaseType::UInt64:
			break;
		case spirv_cross::SPIRType::BaseType::AtomicCounter:
			break;
		case spirv_cross::SPIRType::BaseType::Half:
			break;
		case spirv_cross::SPIRType::BaseType::Float:
			return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::FLOAT) 
				+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::Double:
			return static_cast<crossplatform::VertexType>(static_cast<uint32_t>(crossplatform::VertexType::DOUBLE) 
				+ static_cast<uint32_t>(crossplatform::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::Struct:
			break;
		case spirv_cross::SPIRType::BaseType::Image:
			break;
		case spirv_cross::SPIRType::BaseType::SampledImage:
			break;
		case spirv_cross::SPIRType::BaseType::Sampler:
			break;
		//case spirv_cross::SPIRType::BaseType::AccelerationStructureNV:
		//	break;
		case spirv_cross::SPIRType::BaseType::ControlPointArray:
			break;
		case spirv_cross::SPIRType::BaseType::Char:
			break;
		default:
			break;
		}

		MIRU_ASSERT(true, "ERROR: VULKAN: Unsupported SPIRType::BaseType. Cannot convert to miru::crossplatform::VertexType.");
		return static_cast<crossplatform::VertexType>(0);
	};

	auto sizeof_miru_crossplatform_VertexType =
		[](crossplatform::VertexType type) -> uint32_t
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

	if (stage == spv::ExecutionModel::ExecutionModelVertex)
	{
		m_VSIADs.clear();
		for (auto& res : resources.stage_inputs)
		{
			const spirv_cross::SPIRType& type = compiled_bin.get_type(res.type_id);
			const spirv_cross::SPIRType& base_type = compiled_bin.get_type(res.base_type_id);

			VertexShaderInputAttributeDescription vsiad;
			vsiad.location = compiled_bin.get_decoration(res.id, spv::DecorationLocation);
			vsiad.binding = 0;
			vsiad.vertexType = spirv_cross_SPIRType_BaseType_to_miru_crossplatform_VertexType(type.basetype, type.vecsize);
			vsiad.offset = m_VSIADs.empty() ? 0 : m_VSIADs.back().offset + sizeof_miru_crossplatform_VertexType(m_VSIADs.back().vertexType);
			vsiad.semanticName = res.name;
			m_VSIADs.push_back(vsiad);
		}
	}
	if (stage == spv::ExecutionModel::ExecutionModelFragment)
	{
		m_PSOADs.clear();
		for (auto& res : resources.stage_outputs)
		{
			const spirv_cross::SPIRType& type = compiled_bin.get_type(res.type_id);
			const spirv_cross::SPIRType& base_type = compiled_bin.get_type(res.base_type_id);

			uint32_t location = compiled_bin.get_decoration(res.id, spv::DecorationLocation);
			uint32_t index = compiled_bin.get_decoration(res.id, spv::DecorationIndex);
			uint32_t componenet = compiled_bin.get_decoration(res.id, spv::DecorationComponent);

			FragmentShaderOutputAttributeDescription fsoad;
			fsoad.location = location;
			fsoad.outputType = spirv_cross_SPIRType_BaseType_to_miru_crossplatform_VertexType(type.basetype, type.vecsize);
			fsoad.semanticName = res.name;
			m_PSOADs.push_back(fsoad);
		}
	}

	for (auto& rbds : m_RBDs)
	{
		rbds.second.clear();
	}
	m_RBDs.clear();

	std::vector<std::string> cis_list;
	auto push_back_ResourceBindingDescription = 
		[&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources, crossplatform::DescriptorType descriptorType) -> void
	{
		for (auto& res : resources)
		{
			const spirv_cross::SPIRType& type = compiled_bin.get_type(res.type_id);
			const spirv_cross::SPIRType& base_type = compiled_bin.get_type(res.base_type_id);
			size_t structSize = 0;
			
			crossplatform::DescriptorType descType = descriptorType;
			if(descType == crossplatform::DescriptorType::UNIFORM_BUFFER)
				structSize = compiled_bin.get_declared_struct_size(type);

			uint32_t set = compiled_bin.get_decoration(res.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiled_bin.get_decoration(res.id, spv::DecorationBinding);
			uint32_t descCount = !type.array.empty() ? type.array[0] : 1;

			if (res.name.find("CIS") != std::string::npos)
			{
				bool found = false;
				for (auto& cis : cis_list)
				{
					if (cis.compare(res.name.substr(0, res.name.find_last_of('_'))) == 0)
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
					cis_list.push_back(res.name.substr(0, res.name.find_last_of('_')));
				descType = crossplatform::DescriptorType::COMBINED_IMAGE_SAMPLER;
			}

			ResourceBindingDescription rbd;
			rbd.binding = binding;
			rbd.type = descType;
			rbd.descriptorCount = descCount;
			rbd.stage = m_CI.stage;
			rbd.name = res.name;
			rbd.structSize = structSize;
			m_RBDs[set][binding] = rbd;
		}
	};

	push_back_ResourceBindingDescription(resources.uniform_buffers, crossplatform::DescriptorType::UNIFORM_BUFFER);
	push_back_ResourceBindingDescription(resources.storage_buffers, crossplatform::DescriptorType::STORAGE_BUFFER);
	push_back_ResourceBindingDescription(resources.subpass_inputs, crossplatform::DescriptorType::INPUT_ATTACHMENT);
	push_back_ResourceBindingDescription(resources.storage_images, crossplatform::DescriptorType::STORAGE_IMAGE);
	push_back_ResourceBindingDescription(resources.sampled_images, crossplatform::DescriptorType::SAMPLED_IMAGE);
	//push_back_ResourceBindingDescription(resources.atomic_counters, crossplatform::DescriptorType);
	//push_back_ResourceBindingDescription(resources.acceleration_structures, crossplatform::DescriptorType);
	//push_back_ResourceBindingDescription(resources.push_constant_buffers, crossplatform::DescriptorType);
	push_back_ResourceBindingDescription(resources.separate_images, crossplatform::DescriptorType::SAMPLED_IMAGE);
	push_back_ResourceBindingDescription(resources.separate_samplers, crossplatform::DescriptorType::SAMPLER);
}