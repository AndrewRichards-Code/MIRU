#pragma once
#include "common.h"
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

		std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderTargetDescriptions;
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStenciDescription;

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
		ID3DBlob* m_SerializedRootSignature;
		ID3DBlob* m_SerializedRootSignatureError;
		D3D12_ROOT_SIGNATURE_DESC m_RootSignatureDesc;
		std::vector<D3D12_ROOT_PARAMETER> m_RootParameters;

		ID3D12PipelineState* m_Pipeline;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_GPSD = {};
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_CPSD = {};

		std::vector<D3D12_VIEWPORT> m_Viewports;
		std::vector<D3D12_RECT> m_Scissors;
	};
}
}