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
		//enums/structs
	private:
		struct RootSignature
		{
			ID3D12RootSignature*								rootSignature = nullptr;
			ID3DBlob*											serializedRootSignature = nullptr;
			ID3DBlob*											serializedRootSignatureError = nullptr;
			D3D12_ROOT_SIGNATURE_DESC							rootSignatureDesc;
			std::vector<D3D12_ROOT_PARAMETER>					rootParameters;
			std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>	descriptorRangesSRV_UAV_CBV;
			std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>>	descriptorRangesSampler;
		};

		//Methods
	public:
		Pipeline(Pipeline::CreateInfo* pCreateInfo);
		~Pipeline();

		static D3D12_PRIMITIVE_TOPOLOGY ToD3D12_PRIMITIVE_TOPOLOGY(crossplatform::PrimitiveTopology topology);
		static DXGI_FORMAT ToDXGI_FORMAT(crossplatform::VertexType type);

	private:
		D3D12_BLEND ToD3D12_BLEND(crossplatform::BlendFactor blend);
		D3D12_LOGIC_OP ToD3D12_LOGIC_OP(crossplatform::LogicOp logic);

		RootSignature CreateRootSignature(const crossplatform::Pipeline::PipelineLayout layout, uint32_t setNumOffset = 0, bool localRootSignature = false);
		
		//Members
	public:
		ID3D12Device* m_Device;

		RootSignature m_GlobalRootSignature;

		ID3D12PipelineState* m_Pipeline;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_GPSD = {};
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_CPSD = {};

		ID3D12StateObject* m_RayTracingPipeline;
		D3D12_STATE_OBJECT_DESC m_RayTracingPipelineDesc;
		std::vector<D3D12_STATE_SUBOBJECT> m_RayTracingPipelineSubDesc;
		std::vector<RootSignature> m_LocalRootSignatures;

		std::vector<D3D12_VIEWPORT> m_Viewports;
		std::vector<D3D12_RECT> m_Scissors;
	};
}
}
#endif