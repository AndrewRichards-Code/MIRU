#pragma once
#if defined(MIRU_D3D12)
#include "base/Shader.h"

namespace miru
{
namespace d3d12
{
	class Shader final : public base::Shader
	{
		//Methods
	public:
		Shader(Shader::CreateInfo* pCreateInfo);
		~Shader();

		void Reconstruct() override;
		void GetShaderResources() override;

		void D3D12ShaderReflection(
			const std::vector<char>& shaderBinary,
			const std::vector<std::pair<base::Shader::StageBit, std::string>>& stageAndEntryPoints,
			std::vector<base::Shader::VertexShaderInputAttributeDescription>& VSIADs,
			std::vector<base::Shader::PixelShaderOutputAttributeDescription>& PSOADs,
			std::map<uint32_t, std::map<uint32_t, base::Shader::ResourceBindingDescription>>& RBDs);

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_SHADER_BYTECODE m_ShaderByteCode;
		D3D12_FEATURE_DATA_SHADER_MODEL m_ShaderModelData = {};
	};
}
}
#endif