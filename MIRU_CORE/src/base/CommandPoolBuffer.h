#pragma once
#include "miru_core_common.h"

#include "Pipeline.h"
#include "Buffer.h"
#include "AccelerationStructure.h"

namespace miru
{
namespace base
{
	class MIRU_API CommandPool
	{
		//enums/structs
	public:
		enum class QueueType : uint32_t
		{
			GRAPHICS,
			COMPUTE,
			TRANSFER,
		};
		enum class FlagBit : uint32_t
		{
			TRANSIENT_BIT				= 0x00000001,
			RESET_COMMAND_BUFFER_BIT	= 0x00000002,
			PROTECTED_BIT				= 0x00000004,
		};
		struct CreateInfo
		{
			std::string	debugName;
			ContextRef	context;
			FlagBit		flags;
			QueueType	queueType;
		};
		//Methods
	public:
		static CommandPoolRef Create(CreateInfo* pCreateInfo);
		virtual ~CommandPool() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Trim() = 0;
		virtual void Reset(bool releaseResources) = 0;

		inline const CreateInfo& GetCreateInfo() const { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	class MIRU_API CommandBuffer
	{
		//enums/structs
	public:
		enum class UsageBit : uint32_t
		{
			ONE_TIME_SUBMIT			= 0x00000001,
			RENDEER_PASS_CONTINUE	= 0x00000002,
			SIMULTANEOUS			= 0x00000004
		};
		enum class Level : uint32_t
		{
			PRIMARY,
			SECONDARY
		};
		struct CreateInfo
		{
			std::string		debugName;
			CommandPoolRef	commandPool;
			Level			level;
			uint32_t		commandBufferCount;
		};

		//Methods
	public:
		static CommandBufferRef Create(CreateInfo* pCreateInfo);
		virtual ~CommandBuffer() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Begin(uint32_t index, UsageBit usage) = 0;
		virtual void End(uint32_t index) = 0;
		virtual void Reset(uint32_t index, bool releaseResources) = 0;
		virtual void ExecuteSecondaryCommandBuffers(uint32_t index, const CommandBufferRef& commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices) = 0;
		virtual void Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<SemaphoreRef>& waits, const std::vector<PipelineStageBit>& waitDstPipelineStages, const std::vector<SemaphoreRef>& signals, const FenceRef& fence) = 0;
		virtual void Submit(const std::vector<uint32_t>& cmdBufferIndices, const std::vector<TimelineSemaphoreWithValue>& waits, const std::vector<PipelineStageBit>& waitDstPipelineStages, const std::vector<TimelineSemaphoreWithValue>& signals, const FenceRef& fence, bool unused) = 0;

		virtual void SetEvent(uint32_t index, const EventRef& event, PipelineStageBit pipelineStage) = 0;
		virtual void ResetEvent(uint32_t index, const EventRef& event, PipelineStageBit pipelineStage) = 0;
		virtual void WaitEvents(uint32_t index, const std::vector<EventRef>& events, PipelineStageBit srcStage, PipelineStageBit dstStage, const std::vector<BarrierRef>& barriers) = 0;
		virtual void PipelineBarrier(uint32_t index, PipelineStageBit srcStage, PipelineStageBit dstStage, DependencyBit dependencies, const std::vector<BarrierRef>& barriers) = 0;

		virtual void ClearColourImage(uint32_t index, const ImageRef& image, Image::Layout layout, const Image::ClearColourValue& clear, const std::vector<Image::SubresourceRange>& subresourceRanges) = 0;
		virtual void ClearDepthStencilImage(uint32_t index, const ImageRef& image, Image::Layout layout, const Image::ClearDepthStencilValue& clear, const std::vector<Image::SubresourceRange>& subresourceRanges) = 0;

		virtual void BeginRenderPass(uint32_t index, const FramebufferRef& framebuffer, const std::vector<Image::ClearValue>& clearValues) = 0;
		virtual void EndRenderPass(uint32_t index) = 0;
		virtual void NextSubpass(uint32_t index) = 0;

		virtual void BeginRendering(uint32_t index, const RenderingInfo& renderingInfo) = 0;
		virtual void EndRendering(uint32_t index) = 0;

		virtual void BindPipeline(uint32_t index, const PipelineRef& pipeline) = 0;

		virtual void BindVertexBuffers(uint32_t index, const std::vector<BufferViewRef>& vertexBufferViews) = 0;
		virtual void BindIndexBuffer(uint32_t index, const BufferViewRef& indexBufferView) = 0;

		virtual void BindDescriptorSets(uint32_t index, const std::vector<DescriptorSetRef>& descriptorSets, uint32_t firstSet, const PipelineRef& pipeline) = 0;

		virtual void DrawIndexed(uint32_t index, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;
		virtual void Draw(uint32_t index, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;

		virtual void Dispatch(uint32_t index, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

		virtual void BuildAccelerationStructure(uint32_t index, const std::vector<AccelerationStructureBuildInfoRef>& buildGeometryInfos, const std::vector<std::vector<AccelerationStructureBuildInfo::BuildRangeInfo>>& buildRangeInfos) = 0;
		virtual void TraceRays(uint32_t index, const StridedDeviceAddressRegion* pRaygenShaderBindingTable, const StridedDeviceAddressRegion* pMissShaderBindingTable, const StridedDeviceAddressRegion* pHitShaderBindingTable, const StridedDeviceAddressRegion* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) = 0;

		virtual void CopyBuffer(uint32_t index, const BufferRef& srcBuffer, const BufferRef& dstBuffer, const std::vector<Buffer::Copy>& copyRegions) = 0;
		virtual void CopyImage(uint32_t index, const ImageRef& srcImage, Image::Layout srcImageLayout, const ImageRef& dstImage, Image::Layout dstImageLayout, const std::vector<Image::Copy>& copyRegions) = 0;
		virtual void CopyBufferToImage(uint32_t index, const BufferRef& srcBuffer, const ImageRef& dstImage, Image::Layout dstImageLayout, const std::vector<Image::BufferImageCopy>& regions) = 0;
		virtual void CopyImageToBuffer(uint32_t index, const ImageRef& srcImage, const BufferRef& dstBuffer, Image::Layout srcImageLayout, const std::vector<Image::BufferImageCopy>& regions) = 0;

		virtual void ResolveImage(uint32_t index, const ImageRef& srcImage, Image::Layout srcImageLayout, const ImageRef& dstImage, Image::Layout dstImageLayout, const std::vector<Image::Resolve>& resolveRegions) = 0;

		virtual void BeginDebugLabel(uint32_t index, const std::string& label, std::array<float, 4> rgba = {0.0f, 0.0f , 0.0f, 0.0f }) = 0;
		virtual void EndDebugLabel(uint32_t index) = 0;

		virtual void SetViewport(uint32_t index, const std::vector<Viewport>& viewports) = 0;
		virtual void SetScissor(uint32_t index, const std::vector<Rect2D>& scissors) = 0;

	protected:
		inline bool CheckValidIndex(uint32_t index) { return (index < m_CI.commandBufferCount); }
		#define CHECK_VALID_INDEX_RETURN(index) if (!CheckValidIndex(index)) {return;}

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}