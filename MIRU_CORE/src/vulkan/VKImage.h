#pragma once
#if defined(MIRU_VULKAN)
#include "base/Image.h"

namespace miru
{
namespace vulkan
{
	class Image final : public base::Image
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

	class ImageView final : public base::ImageView
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

	class Sampler final : public base::Sampler
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