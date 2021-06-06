#pragma once
#if defined(MIRU_VULKAN)
#include "crossplatform/CommandPoolBuffer.h"

namespace miru
{
namespace vulkan
{
	class CommandPool final : public crossplatform::CommandPool
	{
		//Methods
	public:
		CommandPool(CommandPool::CreateInfo* pCreateInfo);
		~CommandPool();

		void Trim() override;
		void Reset(bool releaseResources) override;

		uint32_t GetQueueFamilyIndex(const CommandPool::QueueType& type);

		//Members
	public:
		VkDevice& m_Device;
		VkCommandPool m_CmdPool;
		VkCommandPoolCreateInfo m_CmdPoolCI;
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
		void ClearDepthStencilImage(uint32_t index, const Ref<crossplatform::Image>& image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) override;

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
		void CopyImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Copy>& copyRegions) override;
		void CopyBufferToImage(uint32_t index, const Ref<crossplatform::Buffer>& srcBuffer, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions) override;
		void CopyImageToBuffer(uint32_t index, const Ref<crossplatform::Image>& srcImage, const Ref<crossplatform::Buffer>& dstBuffer, crossplatform::Image::Layout srcImageLayout, const std::vector<crossplatform::Image::BufferImageCopy>& regions) override;

		void ResolveImage(uint32_t index, const Ref<crossplatform::Image>& srcImage, crossplatform::Image::Layout srcImageLayout, const Ref<crossplatform::Image>& dstImage, crossplatform::Image::Layout dstImageLayout, const std::vector<crossplatform::Image::Resolve>& resolveRegions) override;

		//Members
	public:
		VkDevice& m_Device;
		VkCommandPool& m_CmdPool;

		std::vector<VkCommandBuffer> m_CmdBuffers;
		VkCommandBufferAllocateInfo m_CmdBufferAI;
		
		std::vector<VkCommandBufferBeginInfo> m_CmdBufferBIs;
		VkSubmitInfo m_CmdBufferSI = {};
	};
}
}
#endif