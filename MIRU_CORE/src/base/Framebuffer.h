#pragma once
#include "miru_core_common.h"

namespace miru
{
namespace base
{
	class MIRU_API Framebuffer
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			std::string					debugName;
			void*						device;
			RenderPassRef				renderPass;
			std::vector<ImageViewRef>	attachments;
			uint32_t					width;
			uint32_t					height;
			uint32_t					layers;
		};

		//Methods
	public:
		static FramebufferRef Create(CreateInfo* pCreateInfo);
		virtual ~Framebuffer() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}