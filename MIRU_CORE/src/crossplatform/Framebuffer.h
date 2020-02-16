#pragma once
#include "common.h"
#include "Pipeline.h"

namespace miru
{
namespace crossplatform
{
	class Framebuffer
	{
		//enum/struct
	public:
		struct CreateInfo
		{
			const char*					debugName;
			void*						device;
			Ref<RenderPass>				renderPass;
			std::vector<Ref<ImageView>>	attachments;
			uint32_t					width;
			uint32_t					height;
			uint32_t					layers;
		};

		//Methods
	public:
		static Ref<Framebuffer> Create(CreateInfo* pCreateInfo);
		virtual ~Framebuffer() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}