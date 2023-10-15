#pragma once
#include "base/Framebuffer.h"
#include "base/DescriptorPoolSet.h"
#include "d3d12/D3D12_Include.h"

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

		base::DescriptorPoolRef m_FramebufferDescriptorPool;
		base::DescriptorPool::CreateInfo m_FramebufferDescriptorPoolCI;

		base::DescriptorSetLayoutRef m_FramebufferDescriptorSetLayout;
		base::DescriptorSetLayout::CreateInfo m_FramebufferDescriptorSetLayoutCI;

		base::DescriptorSetRef m_FramebufferDescriptorSet;
		base::DescriptorSet::CreateInfo m_FramebufferDescriptorSetCI;

		struct ImageView_RTV_DSV_SRV 
		{ 
			base::ImageViewRef imageView; 
			bool HasRTV; bool HasDSV; bool HasSRV;
			bool NeedRTV; bool NeedDSV; bool NeedSRV;
		};
		std::vector<ImageView_RTV_DSV_SRV> m_ImageView_RTV_DSV_SRVs;
	
	};
}
}