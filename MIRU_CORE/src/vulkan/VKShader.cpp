#include "miru_core_common.h"
#if defined(MIRU_VULKAN)
#include "VKShader.h"

#include "base/DescriptorPoolSet.h"

using namespace miru;
using namespace vulkan;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device)), m_ShaderModule(VK_NULL_HANDLE)
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	Reconstruct();
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

	if(m_ShaderModule != VK_NULL_HANDLE)
		vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);

	GetShaderByteCode();

	m_ShaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	m_ShaderModuleCI.pNext = nullptr;
	m_ShaderModuleCI.flags = 0;
	m_ShaderModuleCI.codeSize = static_cast<uint32_t>(m_ShaderBinary.size());
	m_ShaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(m_ShaderBinary.data());

	MIRU_ASSERT(vkCreateShaderModule(m_Device, &m_ShaderModuleCI, nullptr, &m_ShaderModule), "ERROR: VULKAN: Failed to create ShaderModule.");
	VKSetName<VkShaderModule>(m_Device, m_ShaderModule, m_CI.debugName);

	for (const auto& stageAndEntryPoint : m_CI.stageAndEntryPoints)
	{
		const StageBit& stage = stageAndEntryPoint.first;
		const std::string& entryPoint = stageAndEntryPoint.second;

		VkPipelineShaderStageCreateInfo shaderStageCI;
		shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCI.pNext = nullptr;
		shaderStageCI.flags = 0;
		shaderStageCI.stage = static_cast<VkShaderStageFlagBits>(stage);
		shaderStageCI.module = m_ShaderModule;
		shaderStageCI.pName = entryPoint.c_str();
		shaderStageCI.pSpecializationInfo = nullptr;

		m_ShaderStageCIs.push_back(shaderStageCI);
	}
}

void Shader::GetShaderResources()
{
	MIRU_CPU_PROFILE_FUNCTION();

	VulkanShaderReflection(m_ShaderBinary, m_CI.stageAndEntryPoints, m_VSIADs, m_PSOADs, m_GroupCountXYZ, m_RBDs);
}

void Shader::VulkanShaderReflection(
	const std::vector<char>& shaderBinary,
	const std::vector<std::pair<Shader::StageBit, std::string>>& stageAndEntryPoints,
	std::vector<Shader::VertexShaderInputAttributeDescription>& VSIADs,
	std::vector<Shader::PixelShaderOutputAttributeDescription>& PSOADs,
	std::array<uint32_t, 3>& GroupCountXYZ,
	std::map<uint32_t, std::map<uint32_t, Shader::ResourceBindingDescription>>& RBDs)
{
	const uint32_t* spv_bin = reinterpret_cast<const uint32_t*>(shaderBinary.data());
	size_t spv_bin_word_count = shaderBinary.size() / 4;
	spirv_cross::Compiler compiled_bin(spv_bin, spv_bin_word_count);
	spirv_cross::ShaderResources resources = compiled_bin.get_shader_resources();

	Shader::StageBit stageBit = Shader::StageBit(0);
	for (auto& stageAndEntryPoint : stageAndEntryPoints)
		stageBit |= stageAndEntryPoint.first;

	auto spirv_cross_SPIRType_BaseType_to_miru_base_VertexType =
		[](spirv_cross::SPIRType::BaseType type, uint32_t vector_count) -> base::VertexType
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
			return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::INT)
				+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::UInt:
			return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::UINT)
				+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::Int64:
			break;
		case spirv_cross::SPIRType::BaseType::UInt64:
			break;
		case spirv_cross::SPIRType::BaseType::AtomicCounter:
			break;
		case spirv_cross::SPIRType::BaseType::Half:
			break;
		case spirv_cross::SPIRType::BaseType::Float:
			return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::FLOAT)
				+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::Double:
			return static_cast<base::VertexType>(static_cast<uint32_t>(base::VertexType::DOUBLE)
				+ static_cast<uint32_t>(base::VertexType(vector_count - 1)));
		case spirv_cross::SPIRType::BaseType::Struct:
			break;
		case spirv_cross::SPIRType::BaseType::Image:
			break;
		case spirv_cross::SPIRType::BaseType::SampledImage:
			break;
		case spirv_cross::SPIRType::BaseType::Sampler:
			break;
		case spirv_cross::SPIRType::BaseType::AccelerationStructure:
			break;
		case spirv_cross::SPIRType::BaseType::ControlPointArray:
			break;
		case spirv_cross::SPIRType::BaseType::Char:
			break;
		default:
			break;
		}

		MIRU_ASSERT(true, "ERROR: VULKAN: Unsupported SPIRType::BaseType. Cannot convert to miru::base::VertexType.");
		return static_cast<base::VertexType>(0);
	};

	auto sizeof_miru_base_VertexType =
		[](base::VertexType type) -> uint32_t
	{
		switch (type)
		{
		case miru::base::VertexType::FLOAT:
		case miru::base::VertexType::INT:
		case miru::base::VertexType::UINT:
			return 4;
		case miru::base::VertexType::VEC2:
		case miru::base::VertexType::IVEC2:
		case miru::base::VertexType::UVEC2:
			return 8;
		case miru::base::VertexType::VEC3:
		case miru::base::VertexType::IVEC3:
		case miru::base::VertexType::UVEC3:
			return 12;
		case miru::base::VertexType::VEC4:
		case miru::base::VertexType::IVEC4:
		case miru::base::VertexType::UVEC4:
			return 16;
		case miru::base::VertexType::DOUBLE:
			return 8;
		case miru::base::VertexType::DVEC2:
			return 16;
		case miru::base::VertexType::DVEC3:
			return 24;
		case miru::base::VertexType::DVEC4:
			return 32;
		default:
			return 0;
		}
	};

	spv::ExecutionModel executionModel = compiled_bin.get_execution_model();
	if (executionModel == spv::ExecutionModel::ExecutionModelVertex)
	{
		VSIADs.clear();
		for (auto& res : resources.stage_inputs)
		{
			const spirv_cross::SPIRType& type = compiled_bin.get_type(res.type_id);
			const spirv_cross::SPIRType& base_type = compiled_bin.get_type(res.base_type_id);

			Shader::VertexShaderInputAttributeDescription vsiad;
			vsiad.location = compiled_bin.get_decoration(res.id, spv::DecorationLocation);
			vsiad.binding = 0;
			vsiad.vertexType = spirv_cross_SPIRType_BaseType_to_miru_base_VertexType(type.basetype, type.vecsize);
			vsiad.offset = VSIADs.empty() ? 0 : VSIADs.back().offset + sizeof_miru_base_VertexType(VSIADs.back().vertexType);
			vsiad.semanticName = res.name;
			VSIADs.push_back(vsiad);
		}
	}
	if (executionModel == spv::ExecutionModel::ExecutionModelFragment)
	{
		PSOADs.clear();
		for (auto& res : resources.stage_outputs)
		{
			const spirv_cross::SPIRType& type = compiled_bin.get_type(res.type_id);
			const spirv_cross::SPIRType& base_type = compiled_bin.get_type(res.base_type_id);

			uint32_t location = compiled_bin.get_decoration(res.id, spv::DecorationLocation);
			uint32_t index = compiled_bin.get_decoration(res.id, spv::DecorationIndex);
			uint32_t componenet = compiled_bin.get_decoration(res.id, spv::DecorationComponent);

			Shader::FragmentShaderOutputAttributeDescription fsoad;
			fsoad.location = location;
			fsoad.outputType = spirv_cross_SPIRType_BaseType_to_miru_base_VertexType(type.basetype, type.vecsize);
			fsoad.semanticName = res.name;
			PSOADs.push_back(fsoad);
		}
	}
	if (executionModel == spv::ExecutionModel::ExecutionModelGLCompute)
	{
		GroupCountXYZ[0] = compiled_bin.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 0);
		GroupCountXYZ[1] = compiled_bin.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 1);
		GroupCountXYZ[2] = compiled_bin.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 2);
	}
	for (auto& rbds : RBDs)
	{
		rbds.second.clear();
	}
	RBDs.clear();

	std::vector<std::string> cis_list;
	auto push_back_ResourceBindingDescription =
		[&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources, base::DescriptorType descriptorType) -> void
	{
		for (auto& res : resources)
		{
			const spirv_cross::SPIRType& type = compiled_bin.get_type(res.type_id);
			const spirv_cross::SPIRType& base_type = compiled_bin.get_type(res.base_type_id);
			std::string name = compiled_bin.get_name(res.id);

			size_t structSize = 0;
			base::DescriptorType descType = descriptorType;
			if (descType == base::DescriptorType::UNIFORM_BUFFER)
			{
				structSize = compiled_bin.get_declared_struct_size(type);
			}

			uint32_t dimension = 0;
			bool cubemap = false;
			bool array_ = false;
			bool multisample = false;
			bool readwrite = false;
			if (descType == base::DescriptorType::COMBINED_IMAGE_SAMPLER || descType == base::DescriptorType::SAMPLED_IMAGE || descType == base::DescriptorType::STORAGE_IMAGE)
			{
				const spirv_cross::SPIRType::ImageType& image = type.image;
				switch (image.dim)
				{
				case spv::Dim::Dim1D:
					dimension = 1; break;
				case spv::Dim::Dim2D:
					dimension = 2; break;
				case spv::Dim::Dim3D:
					dimension = 3; break;
				case spv::Dim::DimCube:
					dimension = 2; cubemap = true; break;
				default:
					break;
				}
				array_ = image.arrayed;
				multisample = image.ms;
				readwrite = descType == base::DescriptorType::STORAGE_IMAGE ? true : false;
			}

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
					continue;
				else
					cis_list.push_back(res.name.substr(0, res.name.find_last_of('_')));

				descType = base::DescriptorType::COMBINED_IMAGE_SAMPLER;
			}

			Shader::ResourceBindingDescription& rbd = RBDs[set][binding];
			rbd.binding = binding;
			rbd.type = descType;
			rbd.descriptorCount = descCount;
			rbd.stage = stageBit;
			rbd.name = name;
			rbd.structSize = structSize;
			rbd.dimension = dimension;
			rbd.cubemap = cubemap;
			rbd.array_ = array_;
			rbd.multisample = multisample;
			rbd.readwrite = readwrite;
		}
	};

	push_back_ResourceBindingDescription(resources.uniform_buffers, base::DescriptorType::UNIFORM_BUFFER);
	push_back_ResourceBindingDescription(resources.storage_buffers, base::DescriptorType::STORAGE_BUFFER);
	push_back_ResourceBindingDescription(resources.subpass_inputs, base::DescriptorType::INPUT_ATTACHMENT);
	push_back_ResourceBindingDescription(resources.storage_images, base::DescriptorType::STORAGE_IMAGE);
	push_back_ResourceBindingDescription(resources.sampled_images, base::DescriptorType::SAMPLED_IMAGE);
	//push_back_ResourceBindingDescription(resources.atomic_counters, base::DescriptorType);
	push_back_ResourceBindingDescription(resources.acceleration_structures, base::DescriptorType::ACCELERATION_STRUCTURE);
	//push_back_ResourceBindingDescription(resources.push_constant_buffers, base::DescriptorType);
	push_back_ResourceBindingDescription(resources.separate_images, base::DescriptorType::SAMPLED_IMAGE);
	push_back_ResourceBindingDescription(resources.separate_samplers, base::DescriptorType::SAMPLER);
}
#endif