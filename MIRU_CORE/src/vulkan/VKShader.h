#pragma once
#if defined(MIRU_VULKAN)
#include "crossplatform/Shader.h"

namespace miru
{
namespace vulkan
{
	class Shader final : public crossplatform::Shader
	{
		//Methods
	public:
		Shader(Shader::CreateInfo* pCreateInfo);
		~Shader();

		void Reconstruct() override;
		void GetShaderResources() override;

		void VulkanShaderReflection(
			const std::vector<char>& shaderBinary,
			const std::vector<std::pair<crossplatform::Shader::StageBit, std::string>>& stageAndEntryPoints,
			std::vector<crossplatform::Shader::VertexShaderInputAttributeDescription>& VSIADs,
			std::vector<crossplatform::Shader::PixelShaderOutputAttributeDescription>& PSOADs,
			std::map<uint32_t, std::map<uint32_t, crossplatform::Shader::ResourceBindingDescription>>& RBDs);

		//Members
	public:
		VkDevice& m_Device;

		VkShaderModule m_ShaderModule;
		VkShaderModuleCreateInfo m_ShaderModuleCI;

		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStageCIs;
	};
}
}
#endif