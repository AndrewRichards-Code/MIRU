#pragma once
#if defined(MIRU_D3D12)
#include "crossplatform/CommandPoolBuffer.h"

namespace miru
{
namespace d3d12
{
	class CommandPool final : public crossplatform::CommandPool
	{
		//Methods
	public:
		CommandPool(CommandPool::CreateInfo* pCreateInfo);
		~CommandPool();

		void Trim() override;
		void Reset(bool releaseResources) override;

		uint32_t GetCommandQueueIndex(const CommandPool::QueueType& type);

		//Members
	public:
		ID3D12Device* m_Device;
		ID3D12CommandQueue* m_Queue;
		std::vector<ID3D12CommandAllocator*> m_CmdPools;
	};

	class CommandBuffer final : public crossplatform::CommandBuffer
	{
		//Methods
	public:
		CommandBuffer(CommandBuffer::CreateInfo* pCreateInfo);
		~CommandBuffer();

		void Begin(uint32_t index, UsageBit usage) override;
		void End(uint32_t index) override;
		void Reset(uint32_t index, bool releaseResources) override;
		void ExecuteSecondaryCommandBuffers(uint32_t index, const Ref<crossplatform::CommandBuffer>& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices) override;
		void Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<Ref<crossplatform::Semaphore>>& waits, const std::vector<crossplatform::PipelineStageBit>& waitDstPipelineStages, const std::vector<Ref<crossplatform::Semaphore>>& signals, const Ref<crossplatform::Fence>& fence) override;
		void Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<crossplatform::TimelineSemaphoreWithValue>& waits, const std::vector<crossplatform::PipelineStageBit>& waitDstPipelineStages, const std::vector<crossplatform::TimelineSemaphoreWithValue>& signals, const Ref<crossplatform::Fence>& fence) override;

		void SetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage) override;
		void ResetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage) override;
		void WaitEvents(uint32_t, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers) override;
		void PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, crossplatform::DependencyBit dependencies, const std::vector<Ref<crossplatform::Barrier>>& barriers) override;

		void ClearColourImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;
		void ClearDepthStencilImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;

		void BeginRenderPass(uint32_t index, const Ref<crossplatform::Framebuffer>& framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues) override;
		void EndRenderPass(uint32_t index) override;
		void NextSubpass(uint32_t index) override;

		void BeginRendering(uint32_t index, const crossplatform::RenderingInfo& renderingInfo) override;
		void EndRendering(uint32_t index) override;

		void BindPipeline(uint32_t index, const Ref<crossplatform::Pipeline>& pipeline) override;

		void BindVertexBuffers(uint32_t index, const std::vector<Ref<crossplatform::BufferView>>& vertexBufferViews) override;
		void BindIndexBuffer(uint32_t index, const Ref<crossplatform::BufferView>& indexBufferView) override;

		void BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, uint32_t firstSet, const Ref<crossplatform::Pipeline>& pipeline) override;

		void DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
		void Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

		void Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

		void BuildAccelerationStructure(uint32_t index, const std::vector<Ref<crossplatform::AccelerationStructureBuildInfo>>& buildGeometryInfos, const std::vector<std::vector<crossplatform::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos) override;
		void TraceRays(uint32_t index, const crossplatform::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pMissShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pHitShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) override;

		void CopyBuffer(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Buffer>& dstBuffer, const std::vector<crossplatform::Buffer::Copy>& copyRegions) override;
		void CopyImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Copy>& copyRegions) override;
		void CopyBufferToImage(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions) override;
		void CopyImageToBuffer(uint32_t index, const Ref<crossplatform::Image>& srcImage, const Ref<crossplatform::Buffer>& dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions) override;

		void ResolveImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Resolve>& resolveRegions) override;

		void BeginDebugLabel(uint32_t index, const std::string& label, std::array<float, 4> rgba = { 0.0f, 0.0f , 0.0f, 0.0f }) override;
		void EndDebugLabel(uint32_t index) override;

		void SetViewport(uint32_t index, const std::vector<crossplatform::Viewport>& viewports) override;
		void SetScissor(uint32_t index, const std::vector<crossplatform::Rect2D>& scissors) override;

	private:
		void ResolvePreviousSubpassAttachments(uint32_t index);

		//Members
	public:
		ID3D12Device* m_Device;
		std::vector<ID3D12CommandList*> m_CmdBuffers;

	private:
		struct ResourceBindingCapabilities
		{
			uint32_t maxDescriptorCount;
			uint32_t maxCBVsPerStage;
			uint32_t maxSRVsPerStage;
			uint32_t maxUAVsPerStage;
			uint32_t maxSamplersPerStage;
			uint32_t maxSamplerCount = 2048;
		} m_ResourceBindingCapabilities;

		struct RenderingResource
		{
			//End, Reset control
			bool Resettable = false;

			//Descriptor Heap for binding Descriptor Sets
			ID3D12DescriptorHeap* CBV_SRV_UAV_DescriptorHeap;
			ID3D12DescriptorHeap* SAMPLER_DescriptorHeap;
			ID3D12DescriptorHeap* RTV_DescriptorHeap;
			ID3D12DescriptorHeap* DSV_DescriptorHeap;
			UINT CBV_SRV_UAV_DescriptorOffset;
			UINT SAMPLER_DescriptorOffset;
			UINT RTV_DescriptorOffset;
			UINT DSV_DescriptorOffset;
			bool SetDescriptorHeap;

			//RenderPass Control Info
			Ref<crossplatform::Framebuffer> Framebuffer;
			std::vector<crossplatform::Image::ClearValue> ClearValues;
			uint32_t SubpassIndex = (uint32_t)-1;

			//Dynamic Rendering Info
			crossplatform::RenderingInfo RenderingInfo;
		};
		typedef std::vector<RenderingResource> RenderingResources;
		RenderingResources m_RenderingResources;

		std::map<Ref<crossplatform::Image>, crossplatform::Image::Layout> m_RenderPassAttachementImageLayouts;
	};
}
}
#endif