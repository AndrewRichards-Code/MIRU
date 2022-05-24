#include "miru_core_common.h"
#if defined(MIRU_D3D12)
#include "D3D12Framebuffer.h"
#include "D3D12Image.h"
#include "D3D12DescriptorPoolSet.h"

#include "base/Pipeline.h"

using namespace miru;
using namespace d3d12;

Framebuffer::Framebuffer(Framebuffer::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	std::vector<base::DescriptorPool::PoolSize> poolSizes(3);
	poolSizes[0] = { base::DescriptorType::D3D12_RENDER_TARGET_VIEW, 0 };
	poolSizes[1] = { base::DescriptorType::D3D12_DEPTH_STENCIL_VIEW, 0 };
	poolSizes[2] = { base::DescriptorType::SAMPLED_IMAGE, 0 };

	size_t i = 0;
	for (auto& imageView : m_CI.attachments)
	{
		bool HasRTV = false, HasDSV = false, HasSRV = false;
		bool NeedRTV = false, NeedDSV = false, NeedSRV = false;
		const ImageViewRef& d3d12ImageView = ref_cast<ImageView>(imageView);
		if (d3d12ImageView->m_RTVDescHandle.ptr)
			HasRTV = true;
		if (d3d12ImageView->m_DSVDescHandle.ptr)
			HasDSV = true;
		if (d3d12ImageView->m_SRVDescHandle.ptr)
			HasSRV = true;

		Image::Layout initialLayout = m_CI.renderPass->GetCreateInfo().attachments[i].initialLayout;
		Image::Layout finalLayout = m_CI.renderPass->GetCreateInfo().attachments[i].finalLayout;
		auto checkLayout = [&](Image::Layout layout) -> void
		{
			if (layout == Image::Layout::COLOUR_ATTACHMENT_OPTIMAL || layout == Image::Layout::PRESENT_SRC || layout == Image::Layout::SHARED_PRESENT)
				NeedRTV = true;
			if (layout == Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL || layout == Image::Layout::DEPTH_STENCIL_READ_ONLY_OPTIMAL
				|| layout == Image::Layout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL || layout == Image::Layout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
				|| layout == Image::Layout::DEPTH_ATTACHMENT_OPTIMAL || layout == Image::Layout::DEPTH_READ_ONLY_OPTIMAL
				|| layout == Image::Layout::STENCIL_ATTACHMENT_OPTIMAL || layout == Image::Layout::STENCIL_READ_ONLY_OPTIMAL)
				NeedDSV = true;
			if (layout == Image::Layout::SHADER_READ_ONLY_OPTIMAL)
				NeedSRV = true;
		};
		checkLayout(initialLayout);
		checkLayout(finalLayout);

		for (auto& subpassDesc : m_CI.renderPass->GetCreateInfo().subpassDescriptions)
		{
			for (auto& input : subpassDesc.inputAttachments)
			{
				if (input.attachmentIndex == i) 
				{ NeedSRV = true; break; }
			}
			for (auto& colour : subpassDesc.colourAttachments)
			{
				if (colour.attachmentIndex == i) 
				{ NeedRTV = true; break; }
			}
			for (auto& resolve : subpassDesc.resolveAttachments)
			{
				if (resolve.attachmentIndex == i) 
				{ NeedRTV = true; break; }
			}
			for (auto& depthStencil : subpassDesc.depthStencilAttachment)
			{
				if (depthStencil.attachmentIndex == i) 
				{ NeedDSV = true; break; }
			}
		}

		if (NeedRTV && !HasRTV)
			poolSizes[0].descriptorCount++;
		if (NeedDSV && !HasDSV)
			poolSizes[1].descriptorCount++;
		if (NeedSRV && !HasSRV)
			poolSizes[2].descriptorCount++;

		m_ImageView_RTV_DSV_SRVs.push_back({ imageView, HasRTV, HasDSV, HasSRV, NeedRTV, NeedDSV, NeedSRV });
		i++;
	}

	m_FramebufferDescriptorPoolCI.debugName = m_CI.debugName + " : Framebuffer DescriptorPool";
	m_FramebufferDescriptorPoolCI.device = m_Device;
	m_FramebufferDescriptorPoolCI.poolSizes = poolSizes;
	m_FramebufferDescriptorPoolCI.maxSets = 1;
	m_FramebufferDescriptorPool = base::DescriptorPool::Create(&m_FramebufferDescriptorPoolCI);

	std::vector<DescriptorSetLayout::Binding> framebufferDescriptorBindings;
	UINT binding = 0;
	
	for (auto& imageView : m_ImageView_RTV_DSV_SRVs)
	{
		//RTV
		if (imageView.NeedRTV && !imageView.HasRTV)
		{
			framebufferDescriptorBindings.push_back({ binding, base::DescriptorType::D3D12_RENDER_TARGET_VIEW, 1, base::Shader::StageBit::ALL_GRAPHICS });
			binding++;
		}
		//DSV
		else if (imageView.NeedDSV && !imageView.HasDSV)
		{
			framebufferDescriptorBindings.push_back({ binding, base::DescriptorType::D3D12_DEPTH_STENCIL_VIEW, 1, base::Shader::StageBit::ALL_GRAPHICS });
			binding++;
		}
		//SRV
		else if (imageView.NeedSRV && !imageView.HasSRV)
		{
			framebufferDescriptorBindings.push_back({ binding, base::DescriptorType::SAMPLED_IMAGE, 1, base::Shader::StageBit::ALL_GRAPHICS });
			binding++;
		}
	}

	m_FramebufferDescriptorSetLayoutCI.debugName = m_CI.debugName + " : Framebuffer DescriptorSetLayout";
	m_FramebufferDescriptorSetLayoutCI.device = m_Device;
	m_FramebufferDescriptorSetLayoutCI.descriptorSetLayoutBinding = framebufferDescriptorBindings;
	m_FramebufferDescriptorSetLayout = DescriptorSetLayout::Create(&m_FramebufferDescriptorSetLayoutCI);

	m_FramebufferDescriptorSetCI.debugName = m_CI.debugName + " : Framebuffer DescriptorSet";
	m_FramebufferDescriptorSetCI.descriptorPool = m_FramebufferDescriptorPool;
	m_FramebufferDescriptorSetCI.descriptorSetLayouts = { m_FramebufferDescriptorSetLayout };
	m_FramebufferDescriptorSet = base::DescriptorSet::Create(&m_FramebufferDescriptorSetCI);

	binding = 0;
	for (auto& imageView : m_ImageView_RTV_DSV_SRVs)
	{
		//RTV
		if (imageView.NeedRTV && !imageView.HasRTV)
		{
			m_FramebufferDescriptorSet->AddImage(0, binding, { {nullptr, imageView.imageView, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL} });
			imageView.HasRTV = true;
			binding++;
		}
		//DSV
		else if (imageView.NeedDSV && !imageView.HasDSV)
		{
			m_FramebufferDescriptorSet->AddImage(0, binding, { {nullptr, imageView.imageView, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL} });
			imageView.HasDSV = true;
			binding++;
		}
		//SRV
		else if (imageView.NeedSRV && !imageView.HasSRV)
		{
			m_FramebufferDescriptorSet->AddImage(0, binding, { {nullptr, imageView.imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL} });
			imageView.HasSRV = true;
			binding++;
		}
	}

}

Framebuffer::~Framebuffer()
{
	MIRU_CPU_PROFILE_FUNCTION();
}
#endif