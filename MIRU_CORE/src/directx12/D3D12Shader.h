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

		void D3D12ShaderReflection();

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_SHADER_BYTECODE m_ShaderByteCode;
		D3D12_FEATURE_DATA_SHADER_MODEL m_ShaderModelData = {};

	private:
		//DXCompiler Library
		static HMODULE s_HModeuleDXCompiler;
		static std::filesystem::path s_DXCompilerFullpath;
		static uint32_t s_RefCount;
	};
}
}
#endif