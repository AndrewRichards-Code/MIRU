#include "miru_core_common.h"
#if defined (MIRU_D3D12)
#include "d3d12/D3D12Image.h"
#endif
#if defined (MIRU_VULKAN)
#include "vulkan/VKImage.h"
#endif

using namespace miru;
using namespace base;

ImageRef Image::Create(Image::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Image>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Image>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

ImageViewRef ImageView::Create(ImageView::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
	case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::ImageView>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::ImageView>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

SamplerRef Sampler::Create(Sampler::CreateInfo* pCreateInfo)
{
	switch (GraphicsAPI::GetAPI())
	{
		case GraphicsAPI::API::D3D12:
		#if defined (MIRU_D3D12)
		return CreateRef<d3d12::Sampler>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::VULKAN:
		#if defined (MIRU_VULKAN)
		return CreateRef<vulkan::Sampler>(pCreateInfo);
		#else
		return nullptr;
		#endif
	case GraphicsAPI::API::UNKNOWN:
	default:
		MIRU_FATAL(true, "ERROR: BASE: Unknown GraphicsAPI."); return nullptr;
	}
}

Image::FormatData Image::GetFormatData(Image::Format format)
{
	FormatData formatData = {};

	switch (format)
	{
	default:
	case Image::Format::UNKNOWN:
		break;
	case Image::Format::R4G4_UNORM_PACK8:
		formatData = { 4, 4, 0, 0, 0, 0 };
		break;
	case Image::Format::R4G4B4A4_UNORM_PACK16:
	case Image::Format::B4G4R4A4_UNORM_PACK16:
		formatData = { 4, 4, 4, 4, 0, 0 };
		break;
	case Image::Format::R5G6B5_UNORM_PACK16:
	case Image::Format::B5G6R5_UNORM_PACK16:
		formatData = { 5, 6, 5, 0, 0, 0 };
		break;
	case Image::Format::R5G5B5A1_UNORM_PACK16:
	case Image::Format::B5G5R5A1_UNORM_PACK16:
	case Image::Format::A1R5G5B5_UNORM_PACK16:
		formatData = { 5, 5, 5, 1, 0, 0 };
		break;
	case Image::Format::R8_UNORM:
	case Image::Format::R8_SNORM:
	case Image::Format::R8_USCALED:
	case Image::Format::R8_SSCALED:
	case Image::Format::R8_UINT:
	case Image::Format::R8_SINT:
	case Image::Format::R8_SRGB:
		formatData = { 8, 0, 0, 0, 0, 0 };
		break;
	case Image::Format::R8G8_UNORM:
	case Image::Format::R8G8_SNORM:
	case Image::Format::R8G8_USCALED:
	case Image::Format::R8G8_SSCALED:
	case Image::Format::R8G8_UINT:
	case Image::Format::R8G8_SINT:
	case Image::Format::R8G8_SRGB:
		formatData = { 8, 8, 0, 0, 0, 0 };
		break;
	case Image::Format::R8G8B8_UNORM:
	case Image::Format::R8G8B8_SNORM:
	case Image::Format::R8G8B8_USCALED:
	case Image::Format::R8G8B8_SSCALED:
	case Image::Format::R8G8B8_UINT:
	case Image::Format::R8G8B8_SINT:
	case Image::Format::R8G8B8_SRGB:
	case Image::Format::B8G8R8_UNORM:
	case Image::Format::B8G8R8_SNORM:
	case Image::Format::B8G8R8_USCALED:
	case Image::Format::B8G8R8_SSCALED:
	case Image::Format::B8G8R8_UINT:
	case Image::Format::B8G8R8_SINT:
	case Image::Format::B8G8R8_SRGB:
		formatData = { 8, 8, 8, 0, 0, 0 };
		break;
	case Image::Format::R8G8B8A8_UNORM:
	case Image::Format::R8G8B8A8_SNORM:
	case Image::Format::R8G8B8A8_USCALED:
	case Image::Format::R8G8B8A8_SSCALED:
	case Image::Format::R8G8B8A8_UINT:
	case Image::Format::R8G8B8A8_SINT:
	case Image::Format::R8G8B8A8_SRGB:
	case Image::Format::B8G8R8A8_UNORM:
	case Image::Format::B8G8R8A8_SNORM:
	case Image::Format::B8G8R8A8_USCALED:
	case Image::Format::B8G8R8A8_SSCALED:
	case Image::Format::B8G8R8A8_UINT:
	case Image::Format::B8G8R8A8_SINT:
	case Image::Format::B8G8R8A8_SRGB:
	case Image::Format::A8B8G8R8_UNORM_PACK32:
	case Image::Format::A8B8G8R8_SNORM_PACK32:
	case Image::Format::A8B8G8R8_USCALED_PACK32:
	case Image::Format::A8B8G8R8_SSCALED_PACK32:
	case Image::Format::A8B8G8R8_UINT_PACK32:
	case Image::Format::A8B8G8R8_SINT_PACK32:
	case Image::Format::A8B8G8R8_SRGB_PACK32:
		formatData = { 8, 8, 8, 8, 0, 0 };
		break;
	case Image::Format::A2R10G10B10_UNORM_PACK32:
	case Image::Format::A2R10G10B10_SNORM_PACK32:
	case Image::Format::A2R10G10B10_USCALED_PACK32:
	case Image::Format::A2R10G10B10_SSCALED_PACK32:
	case Image::Format::A2R10G10B10_UINT_PACK32:
	case Image::Format::A2R10G10B10_SINT_PACK32:
	case Image::Format::A2B10G10R10_UNORM_PACK32:
	case Image::Format::A2B10G10R10_SNORM_PACK32:
	case Image::Format::A2B10G10R10_USCALED_PACK32:
	case Image::Format::A2B10G10R10_SSCALED_PACK32:
	case Image::Format::A2B10G10R10_UINT_PACK32:
	case Image::Format::A2B10G10R10_SINT_PACK32:
		formatData = { 10, 10, 10, 2, 0, 0 };
		break;
	case Image::Format::R16_UNORM:
	case Image::Format::R16_SNORM:
	case Image::Format::R16_USCALED:
	case Image::Format::R16_SSCALED:
	case Image::Format::R16_UINT:
	case Image::Format::R16_SINT:
	case Image::Format::R16_SFLOAT:
		formatData = { 16, 0, 0, 0, 0, 0 };
		break;
	case Image::Format::R16G16_UNORM:
	case Image::Format::R16G16_SNORM:
	case Image::Format::R16G16_USCALED:
	case Image::Format::R16G16_SSCALED:
	case Image::Format::R16G16_UINT:
	case Image::Format::R16G16_SINT:
	case Image::Format::R16G16_SFLOAT:
		formatData = { 16, 16, 0, 0, 0, 0 };
		break;
	case Image::Format::R16G16B16_UNORM:
	case Image::Format::R16G16B16_SNORM:
	case Image::Format::R16G16B16_USCALED:
	case Image::Format::R16G16B16_SSCALED:
	case Image::Format::R16G16B16_UINT:
	case Image::Format::R16G16B16_SINT:
	case Image::Format::R16G16B16_SFLOAT:
		formatData = { 16, 16, 16, 0, 0, 0 };
		break;
	case Image::Format::R16G16B16A16_UNORM:
	case Image::Format::R16G16B16A16_SNORM:
	case Image::Format::R16G16B16A16_USCALED:
	case Image::Format::R16G16B16A16_SSCALED:
	case Image::Format::R16G16B16A16_UINT:
	case Image::Format::R16G16B16A16_SINT:
	case Image::Format::R16G16B16A16_SFLOAT:
		formatData = { 16, 16, 16, 16, 0, 0 };
		break;
	case Image::Format::R32_UINT:
	case Image::Format::R32_SINT:
	case Image::Format::R32_SFLOAT:
		formatData = { 32, 0, 0, 0, 0, 0 };
		break;
	case Image::Format::R32G32_UINT:
	case Image::Format::R32G32_SINT:
	case Image::Format::R32G32_SFLOAT:
		formatData = { 32, 32, 0, 0, 0, 0 };
		break;
	case Image::Format::R32G32B32_UINT:
	case Image::Format::R32G32B32_SINT:
	case Image::Format::R32G32B32_SFLOAT:
		formatData = { 32, 32, 32, 0, 0, 0 };
		break;
	case Image::Format::R32G32B32A32_UINT:
	case Image::Format::R32G32B32A32_SINT:
	case Image::Format::R32G32B32A32_SFLOAT:
		formatData = { 32, 32, 32, 32, 0, 0 };
		break;
	case Image::Format::R64_UINT:
	case Image::Format::R64_SINT:
	case Image::Format::R64_SFLOAT:
		formatData = { 64, 0, 0, 0, 0, 0 };
		break;
	case Image::Format::R64G64_UINT:
	case Image::Format::R64G64_SINT:
	case Image::Format::R64G64_SFLOAT:
		formatData = { 64, 64, 0, 0, 0, 0 };
		break;
	case Image::Format::R64G64B64_UINT:
	case Image::Format::R64G64B64_SINT:
	case Image::Format::R64G64B64_SFLOAT:
		formatData = { 64, 64, 64, 0, 0, 0 };
		break;
	case Image::Format::R64G64B64A64_UINT:
	case Image::Format::R64G64B64A64_SINT:
	case Image::Format::R64G64B64A64_SFLOAT:
		formatData = { 64, 64, 64, 64, 0, 0 };
		break;
	case Image::Format::B10G11R11_UFLOAT_PACK32:
		formatData = { 10, 11, 11, 0, 0, 0 };
		break;
	case Image::Format::E5B9G9R9_UFLOAT_PACK32:
		formatData = { 9, 9, 9, 5, 0, 0 };
		break;
	case Image::Format::D16_UNORM:
		formatData = { 0, 0, 0, 0, 16, 0 };
		break;
	case Image::Format::X8_D24_UNORM_PACK32:
		formatData = { 0, 0, 0, 0, 24, 8 };
		break;
	case Image::Format::D32_SFLOAT:
		formatData = { 0, 0, 0, 0, 32, 0 };
		break;
	case Image::Format::S8_UINT:
		formatData = { 0, 0, 0, 0, 0, 8 };
		break;
	case Image::Format::D16_UNORM_S8_UINT:
		formatData = { 0, 0, 0, 0, 16, 8 };
		break;
	case Image::Format::D24_UNORM_S8_UINT:
		formatData = { 0, 0, 0, 0, 24, 8 };
		break;
	case Image::Format::D32_SFLOAT_S8_UINT:
		formatData = { 0, 0, 0, 0, 32, 8 };
		break;
	}

	return formatData;
}

uint32_t Image::GetFormatSize(FormatData formatData)
{
	return (formatData.R + formatData.G + formatData.B + formatData.A + formatData.D + formatData.S) / 8;
}

uint32_t Image::GetFormatSize(Image::Format format)
{
	return GetFormatSize(GetFormatData(format));
}

uint32_t Image::GetFormatComponents(FormatData formatData)
{
	return formatData.R > 0 ? 1 : 0
		+ formatData.G > 0 ? 1 : 0
		+ formatData.B > 0 ? 1 : 0
		+ formatData.A > 0 ? 1 : 0
		+ formatData.D > 0 ? 1 : 0
		+ formatData.S > 0 ? 1 : 0;
}

uint32_t Image::GetFormatComponents(Image::Format format)
{
	return GetFormatComponents(GetFormatData(format));
}