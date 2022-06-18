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
		void ExecuteSecondaryCommandBuffers(uint32_t index, const base::CommandBufferRef& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices) override;
		void Submit(const std::vector<base::CommandBuffer::SubmitInfo>& submitInfos, const base::FenceRef& fence) override;
		void Submit2(const std::vector<base::CommandBuffer::SubmitInfo2>& submitInfo2s, const base::FenceRef& fence) override;

		void SetEvent(uint32_t index, const base::EventRef& event, base::PipelineStageBit pipelineStage) override;
		void ResetEvent(uint32_t index, const base::EventRef& event, base::PipelineStageBit pipelineStage) override;
		void WaitEvents(uint32_t, const std::vector<base::EventRef>& events, base::PipelineStageBit srcStage, base::PipelineStageBit dstStage, const std::vector<base::BarrierRef>& barriers) override;
		void PipelineBarrier(uint32_t index, base::PipelineStageBit srcStage, base::PipelineStageBit dstStage, base::DependencyBit dependencies, const std::vector<base::BarrierRef>& barriers) override;
		void PipelineBarrier2(uint32_t index, const base::CommandBuffer::DependencyInfo& dependencyInfo) override;

		void ClearColourImage(uint32_t index, const base::ImageRef& image, base::Image::Layout layout, const base::Image::ClearColourValue& clear, const std::vector<base::Image::SubresourceRange>& subresourceRanges) override;
		void ClearDepthStencilImage(uint32_t index, const base::ImageRef& image, base::Image::Layout layout, const base::Image::ClearDepthStencilValue& clear, const std::vector<base::Image::SubresourceRange>& subresourceRanges) override;

		void BeginRenderPass(uint32_t index, const base::FramebufferRef& framebuffer, const std::vector<base::Image::ClearValue>& clearValues) override;
		void EndRenderPass(uint32_t index) override;
		void NextSubpass(uint32_t index) override;

		void BeginRendering(uint32_t index, const base::RenderingInfo& renderingInfo) override;
		void EndRendering(uint32_t index) override;

		void BindPipeline(uint32_t index, const base::PipelineRef& pipeline) override;

		void BindVertexBuffers(uint32_t index, const std::vector<base::BufferViewRef>& vertexBufferViews) override;
		void BindIndexBuffer(uint32_t index, const base::BufferViewRef& indexBufferView) override;

		void BindDescriptorSets(uint32_t index, const std::vector<base::DescriptorSetRef>& descriptorSets, uint32_t firstSet, const base::PipelineRef& pipeline) override;

		void DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
		void Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;

		void Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

		void BuildAccelerationStructure(uint32_t index, const std::vector<base::AccelerationStructureBuildInfoRef>& buildGeometryInfos, const std::vector<std::vector<base::AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos) override;
		void TraceRays(uint32_t index, const base::StridedDeviceAddressRegion* pRaygenShaderBindingTable, const base::StridedDeviceAddressRegion* pMissShaderBindingTable, const base::StridedDeviceAddressRegion* pHitShaderBindingTable, const base::StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) override;

		void CopyBuffer(uint32_t index, const base::BufferRef& srcBuffer, const base::BufferRef& dstBuffer, const std::vector<base::Buffer::Copy>& copyRegions) override;
		void CopyImage(uint32_t index, const base::ImageRef& srcImage, base::Image::Layout srcImageLayout, const base::ImageRef& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::Copy>& copyRegions) override;
		void CopyBufferToImage(uint32_t index, const base::BufferRef& srcBuffer, const base::ImageRef& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::BufferImageCopy>& regions) override;
		void CopyImageToBuffer(uint32_t index, const base::ImageRef& srcImage, const base::BufferRef& dstBuffer, base::Image::Layout srcImageLayout, const std::vector<base::Image::BufferImageCopy>& regions) override;

		void ResolveImage(uint32_t index, const base::ImageRef& srcImage, base::Image::Layout srcImageLayout, const base::ImageRef& dstImage, base::Image::Layout dstImageLayout, const std::vector<base::Image::Resolve>& resolveRegions) override;

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
			base::FramebufferRef Framebuffer;
			std::vector<base::Image::ClearValue> ClearValues;
			uint32_t SubpassIndex = (uint32_t)-1;

			//Dynamic Rendering Info
			base::RenderingInfo RenderingInfo;
		};
		typedef std::vector<RenderingResource> RenderingResources;
		RenderingResources m_RenderingResources;

		std::map<base::ImageRef, base::Image::Layout> m_RenderPassAttachementImageLayouts;
	};
}
}
#endif