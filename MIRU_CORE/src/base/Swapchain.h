#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace base
{
	class MIRU_API Swapchain
	{
		//enums/structs
	public:
		enum class BPC_ColourSpace
		{
			B8G8R8A8_UNORM_SRGB_NONLINEAR,				// 8-bit sRGB Non-linear
			A2B10G10R10_UNORM_PACK32_SRGB_NONLINEAR,	//10-bit sRGB Non-linear
			A2B10G10R10_UNORM_PACK32_HDR10_ST2084,		//10-bit HDR10 Non-linear
			R16G16B16A16_SFLOAT_EXTENDED_SRGB_LINEAR	//16-bit scRGB Linear
		};
		struct CreateInfo
		{
			std::string		debugName;
			ContextRef		context;
			void*			pWindow;
			uint32_t		width;
			uint32_t		height;
			uint32_t		swapchainCount;
			bool			vSync;
			BPC_ColourSpace bpcColourSpace;
		};

		//Methods
	public:
		static SwapchainRef Create(CreateInfo* pCreateInfo);
		virtual ~Swapchain() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void AcquireNextImage(const SemaphoreRef& acquire, uint32_t& imageIndex) = 0;
		virtual void Present(const CommandPoolRef& cmdPool, const SemaphoreRef& submit, uint32_t& imageIndex) = 0;

	protected:
		void FillSwapchainImageAndViews(void** pImages, void* pImageViews, uint32_t width, uint32_t height, uint32_t format);
		
		//Members
	public:
		std::vector<ImageRef> m_SwapchainImages;
		std::vector<ImageViewRef> m_SwapchainImageViews;

		bool m_Resized = false;

	protected:
		CreateInfo m_CI = {};
	};
}
}