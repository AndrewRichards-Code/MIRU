#pragma once
#if defined(MIRU_D3D12)
#include "miru_core_common.h"
#include "crossplatform/Pipeline.h"

namespace miru
{
namespace d3d12
{
	class RenderPass final : public crossplatform::RenderPass
	{
		//Method
	public:
		RenderPass(RenderPass::CreateInfo* pCreateInfo);
		~RenderPass();

		//Members
	public:
		ID3D12Device* m_Device;
	};

	class Pipeline final : public crossplatform::Pipeline
	{
		//Methods
	public:
		Pipeline(Pipeline::CreateInfo* pCreateInfo);
		~Pipeline();

	private:
		DXGI_FORMAT ToDXGI_FORMAT(crossplatform::VertexType type);
		D3D12_BLEND ToD3D12_BLEND(crossplatform::BlendFactor blend);
		D3D12_LOGIC_OP ToD3D12_LOGIC_OP(crossplatform::LogicOp logic);
		
		//Members
	public:
		ID3D12Device* m_Device;

		ID3D12RootSignature* m_RootSignature;
		ID3DBlob* m_SerializedRootSignature = nullptr;
		ID3DBlob* m_SerializedRootSignatureError = nullptr;
		D3D12_ROOT_SIGNATURE_DESC m_RootSignatureDesc;
		std::vector<D3D12_ROOT_PARAMETER> m_RootParameters;
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_DescriptorRanges;
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_DescriptorRangesSampler;

		ID3D12PipelineState* m_Pipeline;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_GPSD = {};
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_CPSD = {};

		std::vector<D3D12_VIEWPORT> m_Viewports;
		std::vector<D3D12_RECT> m_Scissors;
	};
}
}
#endif