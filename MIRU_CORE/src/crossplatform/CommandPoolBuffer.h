#pragma once
#include "common.h"

#include "Context.h"
#include "Swapchain.h"
#include "Pipeline.h"

namespace miru
{
namespace crossplatform
{
	class CommandPool
	{
		//enums/structs
	public:
		enum class FlagBit : uint32_t
		{
			TRANSIENT_BIT = 0x00000001,
			RESET_COMMAND_BUFFER_BIT = 0x00000002,
			PROTECTED_BIT = 0x00000004,
		};
		struct CreateInfo
		{
			const char*		debugName;
			Ref<Context>	pContext;
			FlagBit			flags;
			uint32_t		queueFamilyIndex;
		};
		//Methods
	public:
		static Ref<CommandPool> Create(CreateInfo* pCreateInfo);
		virtual ~CommandPool() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Trim() = 0;
		virtual void Reset(bool releaseResources) = 0;

		inline const CreateInfo& GetCreateInfo() const { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};

	class CommandBuffer
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
			const char*			debugName;
			Ref<CommandPool>	pCommandPool;
			Level				level;
			uint32_t			commandBufferCount;
		};

		//Methods
	public:
		static Ref<CommandBuffer> Create(CreateInfo* pCreateInfo);
		virtual ~CommandBuffer() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Begin(uint32_t index, UsageBit usage) = 0;
		virtual void End(uint32_t index) = 0;
		virtual void Reset(uint32_t index, bool releaseResources) = 0;
		virtual void ExecuteSecondaryCommandBuffers(uint32_t index, Ref<CommandBuffer> commandBuffer, const std::vector<uint32_t>& secondaryCommandBufferIndices) = 0;
		virtual void Submit(const std::vector<uint32_t>& cmdBufferIndices, std::vector< Ref<Semaphore>>& waits, std::vector< Ref<Semaphore>>& signals, PipelineStageBit pipelineStage, Ref<Fence> fence) = 0;
		virtual void Present(const std::vector<uint32_t>& cmdBufferIndices, Ref<Swapchain> swapchain, std::vector<Ref<Fence>>& draws, std::vector<Ref<Semaphore>>& acquires, std::vector<Ref<Semaphore>>& submits) = 0;

		virtual void SetEvent(uint32_t index, Ref<Event> event, PipelineStageBit pipelineStage) = 0;
		virtual void ResetEvent(uint32_t index, Ref<Event> event, PipelineStageBit pipelineStage) = 0;
		virtual void WaitEvents(uint32_t index, const std::vector<Ref<Event>>& events, PipelineStageBit srcStage, PipelineStageBit dstStage, const std::vector<Ref<Barrier>>& barriers) = 0;
		virtual void PipelineBarrier(uint32_t index, PipelineStageBit srcStage, PipelineStageBit dstStage, const std::vector<Ref<Barrier>>& barriers) = 0;

		virtual void ClearColourImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearColourValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) = 0;
		virtual void ClearDepthStencilImage(uint32_t index, Ref<crossplatform::Image> image, crossplatform::Image::Layout layout, const crossplatform::Image::ClearDepthStencilValue& clear, const std::vector<crossplatform::Image::SubresourceRange>& subresourceRanges) = 0;

	protected:
		inline bool CheckValidIndex(uint32_t index) { return (index < m_CI.commandBufferCount); }
		#define CHECK_VALID_INDEX_RETURN(index) if (!CheckValidIndex(index)) {return;}

		//Members
	protected:
		CreateInfo m_CI = {};

		size_t m_CurrentFrame = 0;
	};
}
}