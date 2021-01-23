#pragma once
#if defined(MIRU_D3D12)
#include "crossplatform/Framebuffer.h"
#include "crossplatform/DescriptorPoolSet.h"

namespace miru
{
namespace d3d12
{
	class Framebuffer final : public crossplatform::Framebuffer
	{
		//Methods
	public:
		Framebuffer(Framebuffer::CreateInfo* pCreateInfo);
		~Framebuffer();

		//Members
	public:
		ID3D12Device* m_Device;

		Ref<crossplatform::DescriptorPool> m_FramebufferDescriptorPool;
		crossplatform::DescriptorPool::CreateInfo m_FramebufferDescriptorPoolCI;

		Ref<crossplatform::DescriptorSetLayout> m_FramebufferDescriptorSetLayout;
		crossplatform::DescriptorSetLayout::CreateInfo m_FramebufferDescriptorSetLayoutCI;

		Ref<crossplatform::DescriptorSet> m_FramebufferDescriptorSet;
		crossplatform::DescriptorSet::CreateInfo m_FramebufferDescriptorSetCI;

		struct ImageView_RTV_DSV_SRV 
		{ 
			Ref<crossplatform::ImageView> imageView; 
			bool HasRTV; bool HasDSV; bool HasSRV;
			bool NeedRTV; bool NeedDSV; bool NeedSRV;
		};
		std::vector<ImageView_RTV_DSV_SRV> m_ImageView_RTV_DSV_SRVs;
	
	};
}
}
#endif