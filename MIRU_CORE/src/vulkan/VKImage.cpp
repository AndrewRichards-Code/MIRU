#include "common.h"
#include "VKImage.h"

using namespace miru;
using namespace vulkan;

Image::Image(Image::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	m_ImageCI.pNext = nullptr;
	m_ImageCI.flags = (m_CI.type == Image::Type::TYPE_CUBE || m_CI.type == Image::Type::TYPE_CUBE_ARRAY) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : (m_CI.type == Image::Type::TYPE_2D_ARRAY) ? VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT : 0;
	m_ImageCI.imageType = (m_CI.type == Image::Type::TYPE_1D) ? VK_IMAGE_TYPE_1D : (m_CI.type == Image::Type::TYPE_2D || m_CI.type == Image::Type::TYPE_CUBE || m_CI.type == Image::Type::TYPE_1D_ARRAY) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
	m_ImageCI.format = static_cast<VkFormat>(m_CI.format);
	m_ImageCI.extent = { m_CI.width, m_CI.height, m_CI.depth };
	m_ImageCI.mipLevels = m_CI.mipLevels;
	m_ImageCI.arrayLayers = m_CI.arrayLayers;
	m_ImageCI.samples = static_cast<VkSampleCountFlagBits>(m_CI.sampleCount);
	m_ImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	m_ImageCI.usage = static_cast<VkImageUsageFlags>(m_CI.usage) + (m_CI.mipLevels > 1 ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0);
	m_ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	m_ImageCI.queueFamilyIndexCount = 0;
	m_ImageCI.pQueueFamilyIndices = nullptr;
	m_ImageCI.initialLayout = static_cast<VkImageLayout>(m_CI.layout);

	MIRU_ASSERT(vkCreateImage(m_Device, &m_ImageCI, nullptr, &m_Image), "ERROR: VULKAN: Failed to create Image.");
	VKSetName<VkImage>(m_Device, (uint64_t)m_Image, m_CI.debugName);

	vkGetImageMemoryRequirements(m_Device, m_Image, &m_MemoryRequirements);

	m_Resource.device = &m_Device;
	m_Resource.type = crossplatform::Resource::Type::IMAGE;
	m_Resource.resource = (uint64_t)m_Image;	
	m_Resource.usage = static_cast<uint32_t>(m_CI.type);
	m_Resource.size = m_MemoryRequirements.size;
	m_Resource.alignment = m_MemoryRequirements.alignment;

	if (m_CI.pMemoryBlock)
	{
		m_CI.pMemoryBlock->AddResource(m_Resource);
		m_CI.pMemoryBlock->SubmitData(m_Resource, m_CI.data);
	}

	m_CurrentLayout = m_ImageCI.initialLayout;
}

Image::~Image()
{
	vkDestroyImage(m_Device, m_Image, nullptr);

	if(m_CI.pMemoryBlock)
		m_CI.pMemoryBlock->RemoveResource(m_Resource.id);
}

void Image::GenerateMipmaps()
{
	/*VkCommandBuffer cmdBuffer = MemoryBlock::GetNextCommandBuffer();
	VkCommandBufferBeginInfo cmdBI;
	cmdBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBI.pNext = nullptr;
	cmdBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cmdBI.pInheritanceInfo = nullptr;
	MIRU_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBI), "ERROR: VULKAN: Failed to begin CommandBuffer.");

	VkImageMemoryBarrier imb;
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.pNext;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = m_Image;
	imb.subresourceRange = { GetVkImageAspect(m_CI.format), 0, 0, 1 };

	int32_t mipWidth = m_CI.width;
	int32_t mipHeight = m_CI.height;

	for (uint32_t i = 0; i < m_CI.mipLevels; i++)
	{
		imb.subresourceRange.baseMipLevel = i;
		imb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &imb);

		VkImageBlit blit = {};
		blit.srcSubresource = { GetVkImageAspect(m_CI.format), i, 0, 1 };
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.dstSubresource = { GetVkImageAspect(m_CI.format), i + 1, 0, 1 };
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

		vkCmdBlitImage(cmdBuffer,
			m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_LINEAR);

		imb.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &imb);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}
	vkEndCommandBuffer(cmdBuffer);*/
}

VkImageAspectFlags Image::GetVkImageAspect(Image::Format format)
{
	switch (format)
	{
	case Image::Format::UNKNOWN:
		return VkImageAspectFlagBits(0);
	case Image::Format::R4G4_UNORM_PACK8:
	case Image::Format::R4G4B4A4_UNORM_PACK16:
	case Image::Format::B4G4R4A4_UNORM_PACK16:
	case Image::Format::R5G6B5_UNORM_PACK16:
	case Image::Format::B5G6R5_UNORM_PACK16:
	case Image::Format::R5G5B5A1_UNORM_PACK16:
	case Image::Format::B5G5R5A1_UNORM_PACK16:
	case Image::Format::A1R5G5B5_UNORM_PACK16:
		//R8
	case Image::Format::R8_UNORM:
	case Image::Format::R8_SNORM:
	case Image::Format::R8_USCALED:
	case Image::Format::R8_SSCALED:
	case Image::Format::R8_UINT:
	case Image::Format::R8_SINT:
	case Image::Format::R8_SRGB:
		//RG8
	case Image::Format::R8G8_UNORM:
	case Image::Format::R8G8_SNORM:
	case Image::Format::R8G8_USCALED:
	case Image::Format::R8G8_SSCALED:
	case Image::Format::R8G8_UINT:
	case Image::Format::R8G8_SINT:
	case Image::Format::R8G8_SRGB:
		//RGB8
	case Image::Format::R8G8B8_UNORM:
	case Image::Format::B8G8R8_UNORM:
	case Image::Format::R8G8B8_SNORM:
	case Image::Format::B8G8R8_SNORM:
	case Image::Format::R8G8B8_USCALED:
	case Image::Format::B8G8R8_USCALED:
	case Image::Format::R8G8B8_SSCALED:
	case Image::Format::B8G8R8_SSCALED:
	case Image::Format::R8G8B8_UINT:
	case Image::Format::B8G8R8_UINT:
	case Image::Format::R8G8B8_SINT:
	case Image::Format::B8G8R8_SINT:
	case Image::Format::R8G8B8_SRGB:
	case Image::Format::B8G8R8_SRGB:
		//RGBA8
	case Image::Format::R8G8B8A8_UNORM:
	case Image::Format::B8G8R8A8_UNORM:
	case Image::Format::A8B8G8R8_UNORM_PACK32:
	case Image::Format::R8G8B8A8_SNORM:
	case Image::Format::B8G8R8A8_SNORM:
	case Image::Format::A8B8G8R8_SNORM_PACK32:
	case Image::Format::R8G8B8A8_USCALED:
	case Image::Format::B8G8R8A8_USCALED:
	case Image::Format::A8B8G8R8_USCALED_PACK32:
	case Image::Format::R8G8B8A8_SSCALED:
	case Image::Format::B8G8R8A8_SSCALED:
	case Image::Format::A8B8G8R8_SSCALED_PACK32:
	case Image::Format::R8G8B8A8_UINT:
	case Image::Format::B8G8R8A8_UINT:
	case Image::Format::A8B8G8R8_UINT_PACK32:
	case Image::Format::R8G8B8A8_SINT:
	case Image::Format::B8G8R8A8_SINT:
	case Image::Format::A8B8G8R8_SINT_PACK32:
	case Image::Format::R8G8B8A8_SRGB:
	case Image::Format::B8G8R8A8_SRGB:
	case Image::Format::A8B8G8R8_SRGB_PACK32:
		//RGB10_A2
	case Image::Format::A2R10G10B10_UNORM_PACK32:
	case Image::Format::A2B10G10R10_UNORM_PACK32:
	case Image::Format::A2R10G10B10_SNORM_PACK32:
	case Image::Format::A2B10G10R10_SNORM_PACK32:
	case Image::Format::A2R10G10B10_USCALED_PACK32:
	case Image::Format::A2B10G10R10_USCALED_PACK32:
	case Image::Format::A2R10G10B10_SSCALED_PACK32:
	case Image::Format::A2B10G10R10_SSCALED_PACK32:
	case Image::Format::A2R10G10B10_UINT_PACK32:
	case Image::Format::A2B10G10R10_UINT_PACK32:
	case Image::Format::A2R10G10B10_SINT_PACK32:
	case Image::Format::A2B10G10R10_SINT_PACK32:
		//R16
	case Image::Format::R16_UNORM:
	case Image::Format::R16_SNORM:
	case Image::Format::R16_USCALED:
	case Image::Format::R16_SSCALED:
	case Image::Format::R16_UINT:
	case Image::Format::R16_SINT:
	case Image::Format::R16_SFLOAT:
	case Image::Format::R16G16_UNORM:
		//RG16
	case Image::Format::R16G16_SNORM:
	case Image::Format::R16G16_USCALED:
	case Image::Format::R16G16_SSCALED:
	case Image::Format::R16G16_UINT:
	case Image::Format::R16G16_SINT:
	case Image::Format::R16G16_SFLOAT:
		//RGB16
	case Image::Format::R16G16B16_UNORM:
	case Image::Format::R16G16B16_SNORM:
	case Image::Format::R16G16B16_USCALED:
	case Image::Format::R16G16B16_SSCALED:
	case Image::Format::R16G16B16_UINT:
	case Image::Format::R16G16B16_SINT:
	case Image::Format::R16G16B16_SFLOAT:
		//RGBA16
	case Image::Format::R16G16B16A16_UNORM:
	case Image::Format::R16G16B16A16_SNORM:
	case Image::Format::R16G16B16A16_USCALED:
	case Image::Format::R16G16B16A16_SSCALED:
	case Image::Format::R16G16B16A16_UINT:
	case Image::Format::R16G16B16A16_SINT:
	case Image::Format::R16G16B16A16_SFLOAT:
		//R32
	case Image::Format::R32_UINT:
	case Image::Format::R32_SINT:
	case Image::Format::R32_SFLOAT:
		//RG32
	case Image::Format::R32G32_UINT:
	case Image::Format::R32G32_SINT:
	case Image::Format::R32G32_SFLOAT:
		//RGB32
	case Image::Format::R32G32B32_UINT:
	case Image::Format::R32G32B32_SINT:
	case Image::Format::R32G32B32_SFLOAT:
		//RGBA32
	case Image::Format::R32G32B32A32_UINT:
	case Image::Format::R32G32B32A32_SINT:
	case Image::Format::R32G32B32A32_SFLOAT:
		//R64, RG64, RGB64, RGBA64
	case Image::Format::R64_UINT:
	case Image::Format::R64_SINT:
	case Image::Format::R64_SFLOAT:
	case Image::Format::R64G64_UINT:
	case Image::Format::R64G64_SINT:
	case Image::Format::R64G64_SFLOAT:
	case Image::Format::R64G64B64_SINT:
	case Image::Format::R64G64B64_SFLOAT:
	case Image::Format::R64G64B64A64_UINT:
	case Image::Format::R64G64B64A64_SINT:
	case Image::Format::R64G64B64A64_SFLOAT:
	case Image::Format::B10G11R11_UFLOAT_PACK32:
	case Image::Format::E5B9G9R9_UFLOAT_PACK32:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	case Image::Format::D16_UNORM:
	case Image::Format::X8_D24_UNORM_PACK32:
	case Image::Format::D32_SFLOAT:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case Image::Format::S8_UINT:
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	case Image::Format::D16_UNORM_S8_UINT:
	case Image::Format::D24_UNORM_S8_UINT:
	case Image::Format::D32_SFLOAT_S8_UINT:
		return VkImageAspectFlagBits(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
	default:
		return VkImageAspectFlagBits(0);
	}
}

ImageView::ImageView(ImageView::CreateInfo* pCreateInfo)
	:m_Device(*reinterpret_cast<VkDevice*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;
	
	m_ImageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_ImageViewCI.pNext = nullptr;
	m_ImageViewCI.flags = 0;
	m_ImageViewCI.image = std::dynamic_pointer_cast<Image>(m_CI.pImage)->m_Image;
	m_ImageViewCI.viewType = static_cast<VkImageViewType>(std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().type);
	m_ImageViewCI.format = static_cast<VkFormat>(std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().format);
	m_ImageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	m_ImageViewCI.subresourceRange = {
		static_cast<VkImageAspectFlags>(m_CI.subresourceRange.aspect),
		m_CI.subresourceRange.baseMipLevel,
		m_CI.subresourceRange.mipLevelCount,
		m_CI.subresourceRange.baseArrayLayer,
		m_CI.subresourceRange.arrayLayerCount,
	};

	MIRU_ASSERT(vkCreateImageView(m_Device, &m_ImageViewCI, nullptr, &m_ImageView), "ERROR: VULKAN: Failed to create ImageView.");
}

ImageView::~ImageView() 
{
	vkDestroyImageView(m_Device, m_ImageView, nullptr);
}
