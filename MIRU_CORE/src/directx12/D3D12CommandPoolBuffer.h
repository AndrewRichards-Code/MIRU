#pragma once
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
		void ExecuteSecondaryCommandBuffers(uint32_t index, Ref<crossplatform::CommandBuffer> commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices) override;
		void Submit(const std::vector<uint32_t>& cmdBufferIndices, std::vector< Ref<crossplatform::Semaphore>>& waits, std::vector<Ref<crossplatform::Semaphore>>& signals, crossplatform::PipelineStageBit pipelineStage, Ref<crossplatform::Fence> fence) override;
		void Present(const std::vector<uint32_t>& cmdBufferIndices, Ref<crossplatform::Swapchain> swapchain, std::vector<Ref<crossplatform::Fence>>& draws, std::vector<Ref<crossplatform::Semaphore>>& acquires, std::vector<Ref<crossplatform::Semaphore>>& submits) override;

		void SetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage) override;
		void ResetEvent(uint32_t index, Ref<crossplatform::Event> event, crossplatform::PipelineStageBit pipelineStage) override;
		void WaitEvents(uint32_t, const std::vector<Ref<crossplatform::Event>>& events, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers) override;
		void PipelineBarrier(uint32_t index, crossplatform::PipelineStageBit srcStage, crossplatform::PipelineStageBit dstStage, const std::vector<Ref<crossplatform::Barrier>>& barriers) override;

		void ClearColourImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;
		void ClearDepthStencilImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;

		void BeginRenderPass(uint32_t index, Ref<crossplatform::Framebuffer> framebuffer, const std::vector<crossplatform::Image::ClearValue>& clearValues) override;
		void EndRenderPass(uint32_t index) override;

		void BindPipeline(uint32_t index, Ref<crossplatform::Pipeline> pipeline) override;

		void BindVertexBuffers(uint32_t index, const std::vector<Ref<crossplatform::BufferView>>& vertexBufferViews) override;
		void BindIndexBuffer(uint32_t index, Ref<crossplatform::BufferView> indexBufferView) override;

		void BindDescriptorSets(uint32_t index, const std::vector<Ref<crossplatform::DescriptorSet>>& descriptorSets, Ref<crossplatform::Pipeline> pipeline) override;

		void DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

		//Members
	public:
		ID3D12Device* m_Device;
		ID3D12CommandAllocator* m_CmdPool;

		std::vector<ID3D12CommandList*> m_CmdBuffers;
	};
}
}