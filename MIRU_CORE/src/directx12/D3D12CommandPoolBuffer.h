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
		ID3D12CommandAllocator* m_CmdPool; 
		ID3D12CommandQueue* m_Queue;
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
		void Present(const std::vector<uint32_t>& cmdBufferIndices, const Ref<crossplatform::Swapchain>& swapchain, const std::vector<Ref<crossplatform::Fence>>& draws, const std::vector<Ref<crossplatform::Semaphore>>& acquires, const std::vector<Ref<crossplatform::Semaphore>>& submits, bool& windowResize) override;

		void SetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage) override;
		void ResetEvent(uint32_t index, const Ref<crossplatform::Event>& event, crossplatform::PipelineStageBit pipelineStage) override;
		void WaitEvents(uint32_t, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers) override;
		void PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, crossplatform::DependencyBit dependencies, const std::vector<Ref<crossplatform::Barrier>>& barriers) override;

		void ClearColourImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;
		void ClearDepthStencilImage(uint32_t index, const Ref<crossplatform::Image> &image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;

		void BeginRenderPass(uint32_t index, const Ref<crossplatform::Framebuffer>& framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues) override;
		void EndRenderPass(uint32_t index) override;
		void NextSubpass(uint32_t index) override;

		void BindPipeline(uint32_t index, const Ref<crossplatform::Pipeline>& pipeline) override;

		void BindVertexBuffers(uint32_t index, const std::vector<Ref<crossplatform::BufferView>>& vertexBufferViews) override;
		void BindIndexBuffer(uint32_t index, const Ref<crossplatform::BufferView>& indexBufferView) override;

		void BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, const Ref<crossplatform::Pipeline>& pipeline) override;

		void DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
		void Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

		void Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

		void BuildAccelerationStructure(uint32_t index, const std::vector<Ref<crossplatform::AccelerationStructureBuildInfo>>& buildGeometryInfos, const std::vector<std::vector<crossplatform::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos) override;
		void TraceRays(uint32_t index, const crossplatform::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pMissShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pHitShaderBindingTable, const crossplatform::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) override;

		void CopyBuffer(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Buffer>& dstBuffer, const std::vector<crossplatform::Buffer::Copy>& copyRegions) override;
		void CopyImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, const Ref<crossplatform::Image>& dstImage, const std::vector<crossplatform::Image::Copy>& copyRegions) override;
		void CopyBufferToImage(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions) override;
		void CopyImageToBuffer(uint32_t index, const Ref<crossplatform::Image>& srcImage, const Ref<crossplatform::Buffer>& dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions) override;

		void ResolveImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Resolve>& resolveRegions) override;
		
	private:
		void ResolvePreviousSubpassAttachments(uint32_t index);

		//Members
	public:
		ID3D12Device* m_Device;
		ID3D12CommandAllocator* m_CmdPool;
		std::vector<ID3D12CommandAllocator*> m_CmdPools;

		std::vector<ID3D12CommandList*> m_CmdBuffers;

	private:
		D3D12_RESOURCE_BINDING_TIER m_ResourceBindingTier;
		uint32_t m_MaxDescriptorCount;
		uint32_t m_MaxCBVsPerStage;
		uint32_t m_MaxSRVsPerStage;
		uint32_t m_MaxUAVsPerStage;
		uint32_t m_MaxSamplersPerStage;
		const uint32_t m_MaxSamplerCount = 2048;

		std::vector<bool> m_SetDescriptorHeaps_PerCmdBuffer;
		UINT m_CBV_SRV_UAV_DescriptorOffset;
		UINT m_SamplerDescriptorOffset;

		ID3D12DescriptorHeap* m_CmdBuffer_CBV_SRV_UAV_DescriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC m_CmdBuffer_CBV_SRV_UAV_DescriptorHeapDesc;
		ID3D12DescriptorHeap* m_CmdBuffer_Sampler_DescriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC m_CmdBuffer_Sampler_DescriptorHeapDesc;

		Ref<crossplatform::Framebuffer> m_RenderPassFramebuffer;
		std::vector<crossplatform::Image::Layout> m_RenderPassFramebufferAttachementLayouts;
		std::vector<crossplatform::Image::ClearValue> m_RenderPassClearValues;
		uint32_t m_SubpassIndex = (uint32_t)-1;
		bool m_Resettable = false;
	};
}
}
#endif