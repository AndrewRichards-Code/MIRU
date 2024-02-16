#pragma once
#include "base/Pipeline.h"
#include "d3d12/D3D12_Include.h"

namespace miru
{
namespace d3d12
{
	class RenderPass final : public base::RenderPass
	{
		//Method
	public:
		RenderPass(RenderPass::CreateInfo* pCreateInfo);
		~RenderPass();

		//Members
	public:
		ID3D12Device* m_Device;
	};

	class Pipeline final : public base::Pipeline
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

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_FLAGS Flags = {};
			CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK NodeMask = {};
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature = {};
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout = {};
			CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE IBStripCutValue = {};
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType = {};
			CD3DX12_PIPELINE_STATE_STREAM_VS VS = {};
			CD3DX12_PIPELINE_STATE_STREAM_GS GS = {};
			CD3DX12_PIPELINE_STATE_STREAM_STREAM_OUTPUT StreamOutput = {};
			CD3DX12_PIPELINE_STATE_STREAM_HS HS = {};
			CD3DX12_PIPELINE_STATE_STREAM_DS DS = {};
			CD3DX12_PIPELINE_STATE_STREAM_PS PS = {};
			CD3DX12_PIPELINE_STATE_STREAM_AS AS = {};
			CD3DX12_PIPELINE_STATE_STREAM_MS MS = {};
			CD3DX12_PIPELINE_STATE_STREAM_CS CS = {};
			CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState = {};
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1 DepthStencilState = {};
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat = {};
			CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState = {};
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats = {};
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc = {};
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask = {};
			CD3DX12_PIPELINE_STATE_STREAM_CACHED_PSO CachedPSO = {};
			CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING ViewInstancingDesc = {};
		};

		//Methods
	public:
		Pipeline(Pipeline::CreateInfo* pCreateInfo);
		~Pipeline();

		std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> GetShaderGroupHandles() override;

		static D3D12_PRIMITIVE_TOPOLOGY ToD3D12_PRIMITIVE_TOPOLOGY(base::PrimitiveTopology topology);
		static DXGI_FORMAT ToDXGI_FORMAT(base::VertexType type);

	private:
		D3D12_BLEND ToD3D12_BLEND(base::BlendFactor blend);
		D3D12_LOGIC_OP ToD3D12_LOGIC_OP(base::LogicOp logic);

		RootSignature CreateRootSignature(const base::Pipeline::PipelineLayout layout, uint32_t setNumOffset = 0, bool localRootSignature = false);
		
		void AddPipelineStateStreamToDesc(size_t offset, size_t size);

		//Members
	public:
		ID3D12Device* m_Device;

		RootSignature m_GlobalRootSignature;
		
		//Graphics and Compute
		ID3D12PipelineState* m_Pipeline = nullptr;
		D3D12_PIPELINE_STATE_STREAM_DESC m_PipelineStateStreamDesc = {};
		std::vector<void*> m_PipelineStateStreamObjects = {};
		PipelineStateStream* m_PipelineStateStream = nullptr;

		std::vector<D3D12_VIEWPORT> m_Viewports = {};
		std::vector<D3D12_RECT> m_Scissors = {};
		std::vector<D3D12_VIEW_INSTANCE_LOCATION> m_ViewInstanceLocations = {};
		
		//RayTracing
		ID3D12StateObject* m_RayTracingPipeline = nullptr;
		D3D12_STATE_OBJECT_DESC m_RayTracingPipelineDesc = {};
		std::vector<D3D12_STATE_SUBOBJECT> m_RayTracingPipelineSubDesc = {};
		std::vector<RootSignature> m_LocalRootSignatures = {};
		std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> m_ShaderGroupHandles = {};
	};
}
}