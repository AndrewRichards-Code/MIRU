#pragma once
#if defined(MIRU_D3D12)
#include "base/CommandPoolBuffer.h"

namespace miru
{
namespace d3d12
{
	class CommandPool final : public base::CommandPool
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

	class CommandBuffer final : public base::CommandBuffer
	{
		//Methods
	public:
		CommandBuffer(CommandBuffer::CreateInfo* pCreateInfo);
		~CommandBuffer();

		void Begin(uint32_t index, UsageBit usage) override;
		void End(uint32_t index) override;
		void Reset(uint32_t index, bool releaseResources) override;
		void ExecuteSecondaryCommandBuffers(uint32_t index, const Ref<base::CommandBuffer>& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices) override;
		void Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<Ref<base::Semaphore>>& waits, const std::vector<base::PipelineStageBit>& waitDstPipelineStages, const std::vector<Ref<base::Semaphore>>& signals, const Ref<base::Fence>& fence) override;
		void Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<base::TimelineSemaphoreWithValue>& waits, const std::vector<base::PipelineStageBit>& waitDstPipelineStages, const std::vector<base::TimelineSemaphoreWithValue>& signals, const Ref<base::Fence>& fence, bool unused) override;

		void SetEvent(uint32_t index, const Ref<base::Event>& event, base::PipelineStageBit pipelineStage) override;
		void ResetEvent(uint32_t index, const Ref<base::Event>& event, base::PipelineStageBit pipelineStage) override;
		void WaitEvents(uint32_t, const std::vector<Ref<base::Event>>& events, base::PipelineStageBit srcStage, base::PipelineStageBit dstStage, const std::vector<Ref<base::Barrier>>& barriers) override;
		void PipelineBarrier(uint32_t index, base::PipelineStageBit srcStage, base::PipelineStageBit dstStage, base::DependencyBit dependencies, const std::vector<Ref<base::Barrier>>& barriers) override;

		void ClearColourImage(uint32_t index, const Ref<base::Image>& image, base::Image::Layout layout, const base::Image::ClearColourValue& clear, const std::vector<base::Image::SubresourceRange>& subresourceRanges) override;
		void ClearDepthStencilImage(uint32_t index, const Ref<base::Image>& image, base::Image::Layout layout, const base::Image::ClearDepthStencilValue& clear, const std::vector<base::Image::SubresourceRange>& subresourceRanges) override;

		void BeginRenderPass(uint32_t index, const Ref<base::Framebuffer>& framebuffer, const std::vector<base::Image::ClearValue>& clearValues) override;
		void EndRenderPass(uint32_t index) override;
		void NextSubpass(uint32_t index) override;

		void BeginRendering(uint32_t index, const base::RenderingInfo& renderingInfo) override;
		void EndRendering(uint32_t index) override;

		void BindPipeline(uint32_t index, const Ref<base::Pipeline>& pipeline) override;

		void BindVertexBuffers(uint32_t index, const std::vector<Ref<base::BufferView>>& vertexBufferViews) override;
		void BindIndexBuffer(uint32_t index, const Ref<base::BufferView>& indexBufferView) override;

		void BindDescriptorSets(uint32_t index, const std::vector<Ref<base::DescriptorSet>>& descriptorSets, uint32_t firstSet, const Ref<base::Pipeline>& pipeline) override;

		void DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
		void Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

		void Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

		void BuildAccelerationStructure(uint32_t index, const std::vector<Ref<base::AccelerationStructureBuildInfo>>& buildGeometryInfos, const std::vector<std::vector<base::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos) override;
		void TraceRays(uint32_t index, const base::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const base::StridedDeviceAddressRegion* pMissShaderBindingTable, const base::StridedDeviceAddressRegion* pHitShaderBindingTable, const base::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) override;

		void CopyBuffer(uint32_t index, const Ref<base::Buffer>& srcBuffer, const Ref<base::Buffer>& dstBuffer, const std::vector<base::Buffer::Copy>& copyRegions) override;
		void CopyImage(uint32_t index, const Ref<base::Image>& srcImage, base::Image::Layout srcImageLayout, const Ref<base::Image>& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::Copy>& copyRegions) override;
		void CopyBufferToImage(uint32_t index, const Ref<base::Buffer>& srcBuffer, const Ref<base::Image>& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::BufferImageCopy>& regions) override;
		void CopyImageToBuffer(uint32_t index, const Ref<base::Image>& srcImage, const Ref<base::Buffer>& dstBuffer, base::Image::Layout srcImageLayout, const std::vector<base::Image::BufferImageCopy>& regions) override;

		void ResolveImage(uint32_t index, const Ref<base::Image>& srcImage, base::Image::Layout srcImageLayout, const Ref<base::Image>& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::Resolve>& resolveRegions) override;

		void BeginDebugLabel(uint32_t index, const std::string& label, std::array<float, 4> rgba = { 0.0f, 0.0f , 0.0f, 0.0f }) override;
		void EndDebugLabel(uint32_t index) override;

		void SetViewport(uint32_t index, const std::vector<base::Viewport>& viewports) override;
		void SetScissor(uint32_t index, const std::vector<base::Rect2D>& scissors) override;

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
			Ref<base::Framebuffer> Framebuffer;
			std::vector<base::Image::ClearValue> ClearValues;
			uint32_t SubpassIndex = (uint32_t)-1;

			//Dynamic Rendering Info
			base::RenderingInfo RenderingInfo;
		};
		typedef std::vector<RenderingResource> RenderingResources;
		RenderingResources m_RenderingResources;

		std::map<Ref<base::Image>, base::Image::Layout> m_RenderPassAttachementImageLayouts;
	};
}
}
#endif