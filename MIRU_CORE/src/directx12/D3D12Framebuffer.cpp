#include "common.h"
#include "D3D12Framebuffer.h"
#include "D3D12Image.h"
#include "D3D12DescriptorPoolSet.h"

using namespace miru;
using namespace d3d12;

Framebuffer::Framebuffer(Framebuffer::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
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
			if (layout == Image::Layout::COLOR_ATTACHMENT_OPTIMAL || layout == Image::Layout::PRESENT_SRC || layout == Image::Layout::SHARED_PRESENT)
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
				if (input.attachmentIndex == i) NeedSRV = true; break;
			for (auto& colour : subpassDesc.colourAttachments)
				if (colour.attachmentIndex == i) NeedRTV = true; break;
			for (auto& resolve : subpassDesc.resolveAttachments)
				if (resolve.attachmentIndex == i) NeedRTV = true; break;
			for (auto& depthStencil : subpassDesc.depthStencilAttachment)
				if (depthStencil.attachmentIndex == i) NeedDSV = true; break;
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

	m_FramebufferDescriptorSetCI.debugName = (std::string(m_CI.debugName) + "Framebuffer DescriptorSet").c_str();
	m_FramebufferDescriptorSetCI.pDescriptorPool = m_FramebufferDescriptorPool;
	m_FramebufferDescriptorSetCI.pDescriptorSetLayouts = {};
	m_FramebufferDescriptorSet = crossplatform::DescriptorSet::Create(&m_FramebufferDescriptorSetCI);

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorWriteLocation;
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT dsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT cbv_srv_uav_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT j = 0;
	for (auto& imageView : m_ImageView_RTV_DSV_SRVs)
	{
		//RTV
		if (imageView.NeedRTV && !imageView.HasRTV)
		{
			descriptorWriteLocation = ref_cast<DescriptorSet>(m_FramebufferDescriptorSet)->m_DescHeapCPUHandles[0][2];
			descriptorWriteLocation.ptr += (rtvDescriptorSize * j);

			m_Device->CreateRenderTargetView(ref_cast<Image>(ref_cast<ImageView>(imageView.imageView)->GetCreateInfo().pImage)->m_Image,
				&ref_cast<ImageView>(imageView.imageView)->m_RTVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(imageView.imageView)->m_RTVDescHandle = descriptorWriteLocation;
			imageView.HasRTV = true;
		}
		//DSV
		else if (imageView.NeedDSV && !imageView.HasDSV)
		{
			descriptorWriteLocation = ref_cast<DescriptorSet>(m_FramebufferDescriptorSet)->m_DescHeapCPUHandles[0][3];
			descriptorWriteLocation.ptr += (dsvDescriptorSize * j);

			m_Device->CreateDepthStencilView(ref_cast<Image>(ref_cast<ImageView>(imageView.imageView)->GetCreateInfo().pImage)->m_Image,
				&ref_cast<ImageView>(imageView.imageView)->m_DSVDesc, descriptorWriteLocation);
			ref_cast<ImageView>(imageView.imageView)->m_DSVDescHandle = descriptorWriteLocation;
			imageView.HasDSV = true;
		}
		//SRV
		else if (imageView.NeedSRV && !imageView.HasSRV)
		{
			descriptorWriteLocation = ref_cast<DescriptorSet>(m_FramebufferDescriptorSet)->m_DescHeapCPUHandles[0][1];
			descriptorWriteLocation.ptr += (cbv_srv_uav_DescriptorSize * j);

			m_Device->CreateShaderResourceView(ref_cast<Image>(ref_cast<ImageView>(imageView.imageView)->GetCreateInfo().pImage)->m_Image,
				/*&ref_cast<ImageView>(imageView.imageView)->m_SRVDesc*/0, descriptorWriteLocation);
			ref_cast<ImageView>(imageView.imageView)->m_SRVDescHandle = descriptorWriteLocation;
			imageView.HasSRV = true;
		}
		j++;
	}

}

Framebuffer::~Framebuffer()
{
}