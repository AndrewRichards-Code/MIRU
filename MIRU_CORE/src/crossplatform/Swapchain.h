#pragma once
#include "common.h"

namespace miru
{
namespace crossplatform
{
	class Context;
	class Swapchain
	{
		//enums/structs
	public:
		struct CreateInfo
		{
			const char*		debugName;
			Ref<Context>	pContext;
			void*			pWindow;
			uint32_t		width;
			uint32_t		height;
			uint32_t		swapchainCount;
			bool			vSync;
		};

		//Methods
	public:
		static Ref<Swapchain> Create(CreateInfo* pCreateInfo);
		virtual ~Swapchain() = default;
		const CreateInfo& GetCreateInfo() { return m_CI; }

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		
		//Members
	protected:
		CreateInfo m_CI = {};
	};
}
}