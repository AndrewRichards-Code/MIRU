#pragma once
#if defined(MIRU_VULKAN)
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

		VmaAllocation m_VmaAllocation;
		VmaAllocationCreateInfo m_VmaACI;
		VmaAllocationInfo m_VmaAI;
	};

	class ImageView final : public crossplatform::ImageView
	{		
		//Methods
	public:
		ImageView() {};
		ImageView(ImageView::CreateInfo* pCreateInfo);
		~ImageView();

		//Members
	public:
		VkDevice m_Device;

		VkImageView m_ImageView;
		VkImageViewCreateInfo m_ImageViewCI = {};
	};

	class Sampler final : public crossplatform::Sampler
	{
		//Methods
	public:
		Sampler(Sampler::CreateInfo* pCreateInfo);
		~Sampler();

		//Members
	public:
		VkDevice& m_Device;

		VkSampler m_Sampler;
		VkSamplerCreateInfo m_SamplerCI = {};
	};
}
}
#endif