#pragma once
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

		//Members
	public:
		VkDevice& m_Device;

		VkShaderModule m_ShaderModule;
		VkShaderModuleCreateInfo m_ShaderModuleCI;

		VkPipelineShaderStageCreateInfo m_ShaderStageCI;
	};
}
}