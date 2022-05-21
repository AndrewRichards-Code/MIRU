#pragma once
#if defined(MIRU_D3D12)
#include "base/Framebuffer.h"
#include "base/DescriptorPoolSet.h"

namespace miru
{
namespace d3d12
{
	class Framebuffer final : public base::Framebuffer
	{
		//Methods
	public:
		Framebuffer(Framebuffer::CreateInfo* pCreateInfo);
		~Framebuffer();

		//Members
	public:
		ID3D12Device* m_Device;

		Ref<base::DescriptorPool> m_FramebufferDescriptorPool;
		base::DescriptorPool::CreateInfo m_FramebufferDescriptorPoolCI;

		Ref<base::DescriptorSetLayout> m_FramebufferDescriptorSetLayout;
		base::DescriptorSetLayout::CreateInfo m_FramebufferDescriptorSetLayoutCI;

		Ref<base::DescriptorSet> m_FramebufferDescriptorSet;
		base::DescriptorSet::CreateInfo m_FramebufferDescriptorSetCI;

		struct ImageView_RTV_DSV_SRV 
		{ 
			Ref<base::ImageView> imageView; 
			bool HasRTV; bool HasDSV; bool HasSRV;
			bool NeedRTV; bool NeedDSV; bool NeedSRV;
		};
		std::vector<ImageView_RTV_DSV_SRV> m_ImageView_RTV_DSV_SRVs;
	
	};
}
}
#endif