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
}

Shader::~Shader()
{
	vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
}