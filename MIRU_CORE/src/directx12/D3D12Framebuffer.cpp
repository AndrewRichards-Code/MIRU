#include "miru_core_common.h"
#include "D3D12Framebuffer.h"
#include "D3D12Image.h"
#include "D3D12DescriptorPoolSet.h"

#include "crossplatform/Pipeline.h"

using namespace miru;
using namespace d3d12;

Framebuffer::Framebuffer(Framebuffer::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	MIRU_CPU_PROFILE_FUNCTION();

	m_CI = *pCreateInfo;

	std::vector<crossplatform::DescriptorPool::PoolSize> poolSizes(3);
	poolSizes[0] = { crossplatform::DescriptorType::D3D12_RENDER_TARGET_VIEW, 0 };
	poolSizes[1] = { crossplatform::DescriptorType::D3D12_DEPTH_STENCIL_VIEW, 0 };
	poolSizes[2] = { crossplatform::DescriptorType::SAMPLED_IMAGE, 0 };

	size_t i = 0;
	for (auto& imageView : m_CI.attachments)
	{
		bool HasRTV = false, HasDSV = false, HasSRV = false;
		bool NeedRTV = false, NeedDSV = false, NeedSRV = false;
		Ref<ImageView> d3d12ImageView = ref_cast<ImageView>(imageView);
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
				|| layout == Image::Layout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL || layout == Image::Layout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)
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

	m_FramebufferDescriptorPoolCI.debugName = (std::string(m_CI.debugName) + "Framebuffer DescriptorPool").c_str();
	m_FramebufferDescriptorPoolCI.device = m_Device;
	m_FramebufferDescriptorPoolCI.poolSizes = poolSizes;
	m_FramebufferDescriptorPoolCI.maxSets = 1;
	m_FramebufferDescriptorPool = crossplatform::DescriptorPool::Create(&m_FramebufferDescriptorPoolCI);

	std::vector<DescriptorSetLayout::Binding> framebufferDescriptorBindings;
	UINT rtvBinding = 0;
	UINT dsvBinding = 0;
	UINT srvBinding = 0;
	for (auto& imageView : m_ImageView_RTV_DSV_SRVs)
	{
		//RTV
		if (imageView.NeedRTV && !imageView.HasRTV)
		{
			framebufferDescriptorBindings.push_back({ rtvBinding , crossplatform::DescriptorType::D3D12_RENDER_TARGET_VIEW, 1, crossplatform::Shader::StageBit::ALL_GRAPHICS });
			rtvBinding++;
		}
		//DSV
		else if (imageView.NeedDSV && !imageView.HasDSV)
		{
			framebufferDescriptorBindings.push_back({ dsvBinding , crossplatform::DescriptorType::D3D12_DEPTH_STENCIL_VIEW, 1, crossplatform::Shader::StageBit::ALL_GRAPHICS });
			dsvBinding++;
		}
		//SRV
		else if (imageView.NeedSRV && !imageView.HasSRV)
		{
			framebufferDescriptorBindings.push_back({ srvBinding , crossplatform::DescriptorType::SAMPLED_IMAGE, 1, crossplatform::Shader::StageBit::ALL_GRAPHICS });
			srvBinding++;
		}
	}

	m_FramebufferDescriptorSetLayoutCI.debugName = "Framebuffer DescriptorSetLayout";
	m_FramebufferDescriptorSetLayoutCI.device = m_Device;
	m_FramebufferDescriptorSetLayoutCI.descriptorSetLayoutBinding = framebufferDescriptorBindings;
	m_FramebufferDescriptorSetLayout = DescriptorSetLayout::Create(&m_FramebufferDescriptorSetLayoutCI);

	m_FramebufferDescriptorSetCI.debugName = (std::string(m_CI.debugName) + "Framebuffer DescriptorSet").c_str();
	m_FramebufferDescriptorSetCI.pDescriptorPool = m_FramebufferDescriptorPool;
	m_FramebufferDescriptorSetCI.pDescriptorSetLayouts = { m_FramebufferDescriptorSetLayout };
	m_FramebufferDescriptorSet = crossplatform::DescriptorSet::Create(&m_FramebufferDescriptorSetCI);

	rtvBinding = 0;
	dsvBinding = 0;
	srvBinding = 0;
	for (auto& imageView : m_ImageView_RTV_DSV_SRVs)
	{
		//RTV
		if (imageView.NeedRTV && !imageView.HasRTV)
		{
			m_FramebufferDescriptorSet->AddImage(0, rtvBinding, { {nullptr, imageView.imageView, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL} });
			imageView.HasRTV = true;
			rtvBinding++;
		}
		//DSV
		else if (imageView.NeedDSV && !imageView.HasDSV)
		{
			m_FramebufferDescriptorSet->AddImage(0, dsvBinding, { {nullptr, imageView.imageView, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL} });
			imageView.HasDSV = true;
			dsvBinding++;
		}
		//SRV
		else if (imageView.NeedSRV && !imageView.HasSRV)
		{
			m_FramebufferDescriptorSet->AddImage(0, srvBinding, { {nullptr, imageView.imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL} });
			imageView.HasSRV = true;
			srvBinding++;
		}
	}

}

Framebuffer::~Framebuffer()
{
	MIRU_CPU_PROFILE_FUNCTION();
}