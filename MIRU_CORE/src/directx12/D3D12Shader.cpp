#include "common.h"
#include "D3D12Shader.h"

using namespace miru;
using namespace d3d12;

Shader::Shader(CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_ShaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_5;
	MIRU_WARN(m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &m_ShaderModelData, sizeof(m_ShaderModelData)), "WARN: D3D12: Unable to CheckFeatureSupport for D3D12_FEATURE_SHADER_MODEL");

	GetShaderByteCode();
	m_ShaderByteCode.pShaderBytecode = m_ShaderBinary.data();
	m_ShaderByteCode.BytecodeLength = m_ShaderBinary.size();
}

Shader::~Shader()
{
	m_ShaderByteCode.pShaderBytecode = nullptr;
	m_ShaderByteCode.BytecodeLength = 0;
}

void Shader::Reconstruct()
{
	GetShaderByteCode();
	m_ShaderByteCode.pShaderBytecode = m_ShaderBinary.data();
	m_ShaderByteCode.BytecodeLength = m_ShaderBinary.size();
}
