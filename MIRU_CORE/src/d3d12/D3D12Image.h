#pragma once
#if defined(MIRU_D3D12)
#include "base/Image.h"

namespace miru
{
namespace d3d12
{
	class Image final : public base::Image
	{
		//Methods
	public:
		Image(Image::CreateInfo* pCreateInfo);
		~Image();

	private:
		void GenerateMipmaps();
		D3D12_RESOURCE_DIMENSION ToD3D12ImageType(Image::Type type) const;

	public:
		static DXGI_FORMAT ToD3D12ImageFormat(Image::Format format);
		static Image::Format ToMIRUImageFormat(DXGI_FORMAT format);
		static D3D12_RESOURCE_STATES ToD3D12ImageLayout(Image::Layout layout);
		inline static UINT D3D12CalculateSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize)
			{ return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize); }

		//Members
	public:
		ID3D12Device* m_Device;

		ID3D12Resource* m_Image;
		D3D12_RESOURCE_DESC m_ResourceDesc;
		D3D12_RESOURCE_STATES m_InitialResourceState; 

		D3D12MA::Allocation* m_D3D12MAllocation;
		D3D12MA::ALLOCATION_DESC m_D3D12MAllocationDesc;
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
		ID3D12Device* m_Device;

		D3D12_RENDER_TARGET_VIEW_DESC m_RTVDesc = {};
		D3D12_DEPTH_STENCIL_VIEW_DESC m_DSVDesc = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc = {};
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_UAVDesc = {};

		D3D12_CPU_DESCRIPTOR_HANDLE m_RTVDescHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_DSVDescHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_UAVDescHandle = {};
	};

	class Sampler final : public base::Sampler
	{
		//Methods
	public:
		Sampler(Sampler::CreateInfo* pCreateInfo);
		~Sampler();

	private:
		D3D12_FILTER ToD3D12Filter(Filter magFilter, Filter minFilter, MipmapMode mipmapMode, bool anisotropic);

		//Members
	public:
		ID3D12Device* m_Device;
		
		D3D12_SAMPLER_DESC m_SamplerDesc;

		D3D12_CPU_DESCRIPTOR_HANDLE m_DescHandle = {};
	};

	inline Image::Format Image::ToMIRUImageFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_UNKNOWN:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return Image::Format::R32G32B32A32_SFLOAT;
		case DXGI_FORMAT_R32G32B32A32_UINT:
			return Image::Format::R32G32B32A32_UINT;
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return Image::Format::R32G32B32A32_SINT;
		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
			return Image::Format::R32G32B32_SFLOAT;
		case DXGI_FORMAT_R32G32B32_UINT:
			return Image::Format::R32G32B32_UINT;
		case DXGI_FORMAT_R32G32B32_SINT:
			return Image::Format::R32G32B32_SINT;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return Image::Format::R16G16B16A16_SFLOAT;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return Image::Format::R16G16B16A16_UNORM;
		case DXGI_FORMAT_R16G16B16A16_UINT:
			return Image::Format::R16G16B16A16_UINT;
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return Image::Format::R16G16B16A16_SNORM;
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return Image::Format::R16G16B16A16_SINT;
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
			return Image::Format::R32G32B32A32_SFLOAT;
		case DXGI_FORMAT_R32G32_UINT:
			return Image::Format::R32G32B32A32_UINT;
		case DXGI_FORMAT_R32G32_SINT:
			return Image::Format::R32G32B32A32_SINT;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return Image::Format::D32_SFLOAT_S8_UINT;
			return Image::Format::D32_SFLOAT_S8_UINT;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return Image::Format::A2B10G10R10_UNORM_PACK32;
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return Image::Format::A2B10G10R10_UINT_PACK32;
		case DXGI_FORMAT_R11G11B10_FLOAT:
			return Image::Format::B10G11R11_UFLOAT_PACK32;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return Image::Format::R8G8B8A8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return Image::Format::R8G8B8A8_SRGB;
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return Image::Format::R8G8B8A8_UINT;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return Image::Format::R8G8B8A8_SNORM;
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return Image::Format::R8G8B8A8_SINT;
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
			return Image::Format::R16G16_SFLOAT;
		case DXGI_FORMAT_R16G16_UNORM:
			return Image::Format::R16G16_UNORM;
		case DXGI_FORMAT_R16G16_UINT:
			return Image::Format::R16G16_UINT;
		case DXGI_FORMAT_R16G16_SNORM:
			return Image::Format::R16G16_SNORM;
		case DXGI_FORMAT_R16G16_SINT:
			return Image::Format::R16G16_SINT;
		case DXGI_FORMAT_R32_TYPELESS:
			return Image::Format::R32_SFLOAT;
		case DXGI_FORMAT_D32_FLOAT:
			return Image::Format::D32_SFLOAT;
		case DXGI_FORMAT_R32_FLOAT:
			return Image::Format::D32_SFLOAT;
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return Image::Format::D24_UNORM_S8_UINT;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
			return Image::Format::R16_SFLOAT;
		case DXGI_FORMAT_D16_UNORM:
			return Image::Format::D16_UNORM;
		case DXGI_FORMAT_R16_UNORM:
			return Image::Format::R16_UNORM;
		case DXGI_FORMAT_R16_UINT:
			return Image::Format::R16_UINT;
		case DXGI_FORMAT_R16_SNORM:
			return Image::Format::R16_SNORM;
		case DXGI_FORMAT_R16_SINT:
			return Image::Format::R16_SINT;
		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
			return Image::Format::R8_UNORM;
		case DXGI_FORMAT_R8_UINT:
			return Image::Format::R8_UINT;
		case DXGI_FORMAT_R8_SNORM:
			return Image::Format::R8_SNORM;
		case DXGI_FORMAT_R8_SINT:
			return Image::Format::R8_SINT;
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R1_UNORM:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			return Image::Format::E5B9G9R9_UFLOAT_PACK32;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_B5G6R5_UNORM:
			return Image::Format::B5G6R5_UNORM_PACK16;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return Image::Format::B5G5R5A1_UNORM_PACK16;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return Image::Format::B8G8R8A8_UNORM;
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return Image::Format::A2B10G10R10_UNORM_PACK32;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return Image::Format::B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return Image::Format::B8G8R8A8_SRGB;
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			return Image::Format::B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return Image::Format::B8G8R8A8_SRGB;
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_YUY2:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
		case DXGI_FORMAT_NV11:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
		case DXGI_FORMAT_A8P8:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return Image::Format::B4G4R4A4_UNORM_PACK16;
		case DXGI_FORMAT_P208:
		case DXGI_FORMAT_V208:
		case DXGI_FORMAT_V408:
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
		case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
			return Image::Format::UNKNOWN;
		case DXGI_FORMAT_A4B4G4R4_UNORM:
			return Image::Format::R4G4B4A4_UNORM_PACK16;
		default:
			return Image::Format::UNKNOWN;
		}
	}
}
}
#endif