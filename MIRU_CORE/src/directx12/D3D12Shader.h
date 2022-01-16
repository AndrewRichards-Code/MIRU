#pragma once
#if defined(MIRU_D3D12)
#include "crossplatform/Shader.h"

namespace miru
{
namespace d3d12
{
	class Shader final : public crossplatform::Shader
	{
		//Methods
	public:
		Shader(Shader::CreateInfo* pCreateInfo);
		~Shader();

		void Reconstruct() override;
		void GetShaderResources() override;

		void D3D12ShaderReflection(
			const std::vector<char>& shaderBinary,
			const std::vector<std::pair<crossplatform::Shader::StageBit, std::string>>& stageAndEntryPoints,
			std::vector<crossplatform::Shader::VertexShaderInputAttributeDescription>& VSIADs,
			std::vector<crossplatform::Shader::PixelShaderOutputAttributeDescription>& PSOADs,
			std::map<uint32_t, std::map<uint32_t, crossplatform::Shader::ResourceBindingDescription>>& RBDs);

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_SHADER_BYTECODE m_ShaderByteCode;
		D3D12_FEATURE_DATA_SHADER_MODEL m_ShaderModelData = {};
	};
}
}
#endif