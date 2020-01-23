#pragma once
#include "crossplatform/Image.h"

namespace miru
{
namespace vulkan
{
	class Image final : public crossplatform::Image
	{
		//Methods
	public:
		Image() {};
		Image(Image::CreateInfo* pCreateInfo);
		~Image();

	private:
		void GenerateMipmaps();
		VkImageAspectFlags GetVkImageAspect(Image::Format format);

		//Members
	public:
		VkDevice m_Device;

		VkImage m_Image;
		VkImageCreateInfo m_ImageCI = {};
		VkMemoryRequirements m_MemoryRequirements;

		VkImageLayout m_CurrentLayout;
	};

	class ImageView final : public crossplatform::ImageView
	{		
		//Methods
	public:
		ImageView(ImageView::CreateInfo* pCreateInfo);
		~ImageView();

		//Members
	public:
		VkDevice& m_Device;

		VkImageView m_ImageView;
		VkImageViewCreateInfo m_ImageViewCI = {};
	};
}
}