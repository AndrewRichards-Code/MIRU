#pragma once
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

		//Members
	public:
		ID3D12Device* m_Device;

		D3D12_SHADER_BYTECODE m_ShaderByteCode;
	};
}
}
