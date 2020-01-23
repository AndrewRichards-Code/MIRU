#include "common.h"
#include "D3D12Shader.h"

using namespace miru;
using namespace d3d12;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	GetShaderByteCode();
	m_ShaderByteCode.pShaderBytecode = m_ShaderBinary.data();
	m_ShaderByteCode.BytecodeLength = m_ShaderBinary.size();
}

Shader::~Shader()
{
	m_ShaderByteCode.pShaderBytecode = nullptr;
	m_ShaderByteCode.BytecodeLength = 0;
}