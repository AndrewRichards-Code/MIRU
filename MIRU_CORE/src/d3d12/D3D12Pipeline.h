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
		
		//Members
	public:
		ID3D12Device* m_Device;

		RootSignature m_GlobalRootSignature;

		ID3D12PipelineState* m_Pipeline = nullptr;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_GPSD = {};
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_CPSD = {};

		ID3D12StateObject* m_RayTracingPipeline = nullptr;
		D3D12_STATE_OBJECT_DESC m_RayTracingPipelineDesc;
		std::vector<D3D12_STATE_SUBOBJECT> m_RayTracingPipelineSubDesc;
		std::vector<RootSignature> m_LocalRootSignatures;

		std::vector<D3D12_VIEWPORT> m_Viewports;
		std::vector<D3D12_RECT> m_Scissors;

		std::vector<std::pair<base::ShaderGroupHandleType, std::vector<uint8_t>>> m_ShaderGroupHandles;

		D3D12_VIEW_INSTANCING_DESC m_ViewInstancingDesc;
		std::vector<D3D12_VIEW_INSTANCE_LOCATION> m_ViewInstanceLocations;

		D3D12_SHADER_BYTECODE AS = {};
		D3D12_SHADER_BYTECODE MS = {};
	};
}
}