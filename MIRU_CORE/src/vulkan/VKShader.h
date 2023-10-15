#pragma once
#include "base/Shader.h"
#include "vulkan/VK_Include.h"

namespace miru
{
namespace vulkan
{
	class Shader final : public base::Shader
	{
		//Methods
	public:
		Shader(Shader::CreateInfo* pCreateInfo);
		~Shader();

		void Reconstruct() override;
		void GetShaderResources() override;

		void VulkanShaderReflection(
			const std::vector<char>& shaderBinary,
			const std::vector<std::pair<base::Shader::StageBit, std::string>>& stageAndEntryPoints,
			std::vector<base::Shader::VertexShaderInputAttributeDescription>& VSIADs,
			std::vector<base::Shader::PixelShaderOutputAttributeDescription>& PSOADs,
			std::array<uint32_t, 3>& ThreadGroupSizeXYZ,
			std::map<uint32_t, std::map<uint32_t, base::Shader::ResourceBindingDescription>>& RBDs);

		//Members
	public:
		VkDevice& m_Device;

		VkShaderModule m_ShaderModule;
		VkShaderModuleCreateInfo m_ShaderModuleCI;

		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStageCIs;
	};
}
}