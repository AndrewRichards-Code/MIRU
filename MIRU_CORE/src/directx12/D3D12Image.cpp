#include "common.h"
#include "D3D12Image.h"
#include "D3D12Allocator.h"

using namespace miru;
using namespace d3d12;

Image::Image(Image::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	m_ResourceDesc.Dimension = ToD3D12ImageType(m_CI.type);
	m_ResourceDesc.Alignment = 0;
	m_ResourceDesc.Width = m_CI.width;
	m_ResourceDesc.Height = m_CI.height;
	m_ResourceDesc.DepthOrArraySize = m_CI.type == Image::Type::TYPE_3D ? m_CI.depth : m_CI.arrayLayers;
	m_ResourceDesc.MipLevels = m_CI.mipLevels;
	m_ResourceDesc.Format = ToD3D12ImageFormat(m_CI.format);
	m_ResourceDesc.SampleDesc.Count = static_cast<UINT>(m_CI.sampleCount);
	m_ResourceDesc.SampleDesc.Quality = 0;
	m_ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	m_ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_CLEAR_VALUE* clear = nullptr;
	m_CurrentResourceState = ToD3D12ImageLayout(m_CI.layout);

	D3D12_RESOURCE_ALLOCATION_INFO ai = m_Device->GetResourceAllocationInfo(0, 1, &m_ResourceDesc);

	m_Resource.device = m_Device;
	m_Resource.type = crossplatform::Resource::Type::BUFFER;
	m_Resource.resource = (uint64_t)m_Image; // This image handle is invalid, it's assigned after the ID3D12Device::CreatePlacedResource()
	m_Resource.usage = static_cast<uint32_t>(m_CI.usage);;
	m_Resource.size = ai.SizeInBytes;
	m_Resource.alignment = ai.Alignment;

	if (m_CI.pMemoryBlock)
	{
		m_CI.pMemoryBlock->AddResource(m_Resource);

		MIRU_ASSERT(m_Device->CreatePlacedResource((ID3D12Heap*)m_Resource.memoryBlock, m_Resource.offset, &m_ResourceDesc, m_CurrentResourceState, clear, IID_PPV_ARGS(&m_Image)), "ERROR: D3D12: Failed to place Image.");
		D3D12SetName(m_Image, m_CI.debugName);

		m_Resource.resource = (uint64_t)m_Image;
		m_CI.pMemoryBlock->GetAllocatedResources().at(m_CI.pMemoryBlock.get()).at(m_Resource.id).resource = (uint64_t)m_Image;
		m_CI.pMemoryBlock->SubmitData(m_Resource, m_CI.data);
	}
}

Image::~Image()
{
	SAFE_RELEASE(m_Image);

	if (m_CI.pMemoryBlock)
		m_CI.pMemoryBlock->RemoveResource(m_Resource.id);
}

void Image::GenerateMipmaps()
{
}

D3D12_RESOURCE_DIMENSION Image::ToD3D12ImageType(Image::Type type) const
{
	switch (type)
	{
	case Image::Type::TYPE_1D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case Image::Type::TYPE_2D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case Image::Type::TYPE_3D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	case Image::Type::TYPE_CUBE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case Image::Type::TYPE_1D_ARRAY:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case Image::Type::TYPE_2D_ARRAY:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case Image::Type::TYPE_CUBE_ARRAY:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	default:
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

DXGI_FORMAT Image::ToD3D12ImageFormat(Image::Format format) const
{
	switch (format)
	{
	case Image::Format::UNKNOWN:
	case Image::Format::R4G4_UNORM_PACK8:
	case Image::Format::R4G4B4A4_UNORM_PACK16:
	case Image::Format::B4G4R4A4_UNORM_PACK16:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R5G6B5_UNORM_PACK16:
	case Image::Format::B5G6R5_UNORM_PACK16:
		return DXGI_FORMAT_B5G6R5_UNORM;
	case Image::Format::R5G5B5A1_UNORM_PACK16:
	case Image::Format::B5G5R5A1_UNORM_PACK16:
	case Image::Format::A1R5G5B5_UNORM_PACK16:
		return DXGI_FORMAT_B5G5R5A1_UNORM;
		//R8
	case Image::Format::R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;
	case Image::Format::R8_SNORM:
		return DXGI_FORMAT_R8_SNORM;
	case Image::Format::R8_USCALED:
	case Image::Format::R8_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case Image::Format::R8_SINT:
		return DXGI_FORMAT_R8_SINT;
	case Image::Format::R8_SRGB:
		return DXGI_FORMAT_UNKNOWN;
		//RG8
	case Image::Format::R8G8_UNORM:
		return DXGI_FORMAT_R8G8_UNORM;
	case Image::Format::R8G8_SNORM:
		return DXGI_FORMAT_R8G8_SNORM;
	case Image::Format::R8G8_USCALED:
	case Image::Format::R8G8_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R8G8_UINT:
		return DXGI_FORMAT_R8G8_UINT;
	case Image::Format::R8G8_SINT:
		return DXGI_FORMAT_R8G8_SINT;
	case Image::Format::R8G8_SRGB:
		return DXGI_FORMAT_UNKNOWN;
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
		return DXGI_FORMAT_UNKNOWN;
		//RGBA8
	case Image::Format::R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case Image::Format::B8G8R8A8_UNORM:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case Image::Format::A8B8G8R8_UNORM_PACK32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case Image::Format::R8G8B8A8_SNORM:
	case Image::Format::B8G8R8A8_SNORM:
	case Image::Format::A8B8G8R8_SNORM_PACK32:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case Image::Format::R8G8B8A8_USCALED:
	case Image::Format::B8G8R8A8_USCALED:
	case Image::Format::A8B8G8R8_USCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R8G8B8A8_SSCALED:
	case Image::Format::B8G8R8A8_SSCALED:
	case Image::Format::A8B8G8R8_SSCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R8G8B8A8_UINT:
	case Image::Format::B8G8R8A8_UINT:
	case Image::Format::A8B8G8R8_UINT_PACK32:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case Image::Format::R8G8B8A8_SINT:
	case Image::Format::B8G8R8A8_SINT:
	case Image::Format::A8B8G8R8_SINT_PACK32:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case Image::Format::R8G8B8A8_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case Image::Format::B8G8R8A8_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case Image::Format::A8B8G8R8_SRGB_PACK32:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//RGB10_A2
	case Image::Format::A2R10G10B10_UNORM_PACK32:
	case Image::Format::A2B10G10R10_UNORM_PACK32:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case Image::Format::A2R10G10B10_SNORM_PACK32:
	case Image::Format::A2B10G10R10_SNORM_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::A2R10G10B10_USCALED_PACK32:
	case Image::Format::A2B10G10R10_USCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::A2R10G10B10_SSCALED_PACK32:
	case Image::Format::A2B10G10R10_SSCALED_PACK32:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::A2R10G10B10_UINT_PACK32:
	case Image::Format::A2B10G10R10_UINT_PACK32:
		return DXGI_FORMAT_R10G10B10A2_UINT;
	case Image::Format::A2R10G10B10_SINT_PACK32:
	case Image::Format::A2B10G10R10_SINT_PACK32:
		return DXGI_FORMAT_UNKNOWN;
		//R16
	case Image::Format::R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case Image::Format::R16_SNORM:
		return DXGI_FORMAT_D16_UNORM;
	case Image::Format::R16_USCALED:
	case Image::Format::R16_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R16_UINT:
		return DXGI_FORMAT_R16_UINT;
	case Image::Format::R16_SINT:
		return DXGI_FORMAT_R16_SINT;
	case Image::Format::R16_SFLOAT:
		return DXGI_FORMAT_R16_FLOAT;
		//RG16
	case Image::Format::R16G16_UNORM:
		return DXGI_FORMAT_R16G16_UNORM;
	case Image::Format::R16G16_SNORM:
		return DXGI_FORMAT_R16G16_SNORM;
	case Image::Format::R16G16_USCALED:
	case Image::Format::R16G16_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R16G16_UINT:
		return DXGI_FORMAT_R16G16_UINT;
	case Image::Format::R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case Image::Format::R16G16_SFLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
		//RGB16
	case Image::Format::R16G16B16_UNORM:
	case Image::Format::R16G16B16_SNORM:
	case Image::Format::R16G16B16_USCALED:
	case Image::Format::R16G16B16_SSCALED:
	case Image::Format::R16G16B16_UINT:
	case Image::Format::R16G16B16_SINT:
	case Image::Format::R16G16B16_SFLOAT:
		return DXGI_FORMAT_UNKNOWN;
		//RGBA16
	case Image::Format::R16G16B16A16_UNORM:
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case Image::Format::R16G16B16A16_SNORM:
		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case Image::Format::R16G16B16A16_USCALED:
	case Image::Format::R16G16B16A16_SSCALED:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::R16G16B16A16_UINT:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case Image::Format::R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case Image::Format::R16G16B16A16_SFLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
		//R32
	case Image::Format::R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case Image::Format::R32_SINT:
		return DXGI_FORMAT_R32_SINT;
	case Image::Format::R32_SFLOAT:
		return DXGI_FORMAT_R32_FLOAT;
		//RG32
	case Image::Format::R32G32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case Image::Format::R32G32_SINT:
		return DXGI_FORMAT_R32G32_SINT;
	case Image::Format::R32G32_SFLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
		//RGB32
	case Image::Format::R32G32B32_UINT:
		return DXGI_FORMAT_R32G32B32_UINT;
	case Image::Format::R32G32B32_SINT:
		return DXGI_FORMAT_R32G32B32_SINT;
	case Image::Format::R32G32B32_SFLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		//RGBA32
	case Image::Format::R32G32B32A32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case Image::Format::R32G32B32A32_SINT:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case Image::Format::R32G32B32A32_SFLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
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
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::B10G11R11_UFLOAT_PACK32:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case Image::Format::E5B9G9R9_UFLOAT_PACK32:
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case Image::Format::D16_UNORM:
		return DXGI_FORMAT_D16_UNORM;
	case Image::Format::X8_D24_UNORM_PACK32:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case Image::Format::D32_SFLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	case Image::Format::S8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case Image::Format::D16_UNORM_S8_UINT:
		return DXGI_FORMAT_UNKNOWN;
	case Image::Format::D24_UNORM_S8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case Image::Format::D32_SFLOAT_S8_UINT:
		return DXGI_FORMAT_UNKNOWN;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_RESOURCE_STATES Image::ToD3D12ImageLayout(Image::Layout layout)
{
	switch (layout)
	{
		case Image::Layout::UNKNOWN:
			return D3D12_RESOURCE_STATE_COMMON;
		case Image::Layout::GENERAL:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		case Image::Layout::COLOR_ATTACHMENT_OPTIMAL:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ;
		case Image::Layout::DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case Image::Layout::SHADER_READ_ONLY_OPTIMAL:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		case Image::Layout::TRANSFER_SRC_OPTIMAL:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case Image::Layout::TRANSFER_DST_OPTIMAL:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case Image::Layout::PREINITIALIZED:
			return D3D12_RESOURCE_STATE_COMMON;
		case Image::Layout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case Image::Layout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case Image::Layout::PRESENT_SRC:
			return D3D12_RESOURCE_STATE_PRESENT;
		case Image::Layout::SHARED_PRESENT:
			return D3D12_RESOURCE_STATE_PRESENT;
		default:
			return D3D12_RESOURCE_STATE_COMMON;
	}
}

ImageView::ImageView(ImageView::CreateInfo* pCreateInfo)
	:m_Device(reinterpret_cast<ID3D12Device*>(pCreateInfo->device))
{
	m_CI = *pCreateInfo;

	D3D12_RESOURCE_DESC resourceDesc = std::dynamic_pointer_cast<Image>(m_CI.pImage)->m_ResourceDesc;
	ID3D12Resource* image = std::dynamic_pointer_cast<Image>(m_CI.pImage)->m_Image;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = {0};

	//RTV
	UINT rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (uint32_t i = m_CI.subresourceRange.baseMipLevel; i < m_CI.subresourceRange.mipLevelCount; i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = resourceDesc.Format;

		switch (std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().type)
		{
		case Image::Type::TYPE_1D:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			rtvDesc.Texture1D.MipSlice = i;
			break;
		}
		case Image::Type::TYPE_2D:
		{
			if (resourceDesc.SampleDesc.Count > 1)
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
				break;
			}
			else
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = i;
				rtvDesc.Texture2D.PlaneSlice = 0;
				break;
			}
		}
		case  Image::Type::TYPE_3D:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			rtvDesc.Texture3D.FirstWSlice = m_CI.subresourceRange.baseArrayLayer;
			rtvDesc.Texture3D.WSize = m_CI.subresourceRange.arrayLayerCount;
			rtvDesc.Texture3D.MipSlice = i;
			break;
		}
		case Image::Type::TYPE_1D_ARRAY:
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			rtvDesc.Texture1DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
			rtvDesc.Texture1DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
			rtvDesc.Texture1DArray.MipSlice = i;
			break;
		}
		case Image::Type::TYPE_CUBE:
		case Image::Type::TYPE_CUBE_ARRAY:
		case Image::Type::TYPE_2D_ARRAY:
		{
			if (resourceDesc.SampleDesc.Count > 1)
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
				rtvDesc.Texture2DMSArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
				rtvDesc.Texture2DMSArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
				break;
			}
			else
			{
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
				rtvDesc.Texture2DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
				rtvDesc.Texture2DArray.MipSlice = i;
				rtvDesc.Texture2DArray.PlaneSlice = 0;
				break;
			}
		}
		}
		m_Device->CreateRenderTargetView(image, &rtvDesc, descriptorHandle);
		descriptorHandle.ptr += rtvDescriptorSize;
	}

	//DSV
	if(std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().format >= Image::Format::D16_UNORM)
	{
		UINT dsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		for (uint32_t i = m_CI.subresourceRange.baseMipLevel; i < m_CI.subresourceRange.mipLevelCount; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Format = resourceDesc.Format;
			dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;

			switch (std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().type)
			{
			case Image::Type::TYPE_1D:
			{
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
				dsvDesc.Texture1D.MipSlice = i;
				break;
			}
			case Image::Type::TYPE_2D:
			{
				if (resourceDesc.SampleDesc.Count > 1)
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
					break;
				}
				else
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = i;
					break;
				}
			}
			case Image::Type::TYPE_1D_ARRAY:
			{
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
				dsvDesc.Texture1DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
				dsvDesc.Texture1DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
				dsvDesc.Texture1DArray.MipSlice = i;
				break;
			}
			case Image::Type::TYPE_3D:
			case Image::Type::TYPE_CUBE:
			case Image::Type::TYPE_CUBE_ARRAY:
			case Image::Type::TYPE_2D_ARRAY:
			{
				if (resourceDesc.SampleDesc.Count > 1)
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
					dsvDesc.Texture2DMSArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
					dsvDesc.Texture2DMSArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
					break;
				}
				else
				{
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
					dsvDesc.Texture2DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
					dsvDesc.Texture2DArray.MipSlice = i;
					break;
				}
			}
			}
			m_Device->CreateDepthStencilView(image, &dsvDesc, descriptorHandle);
			descriptorHandle.ptr += dsvDescriptorSize;
		}
	}

	//SRV
	UINT srvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = resourceDesc.Format;

		switch (std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().type)
		{
		case Image::Type::TYPE_1D:
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			srvDesc.Texture1D.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;
			srvDesc.Texture1D.MipLevels = m_CI.subresourceRange.mipLevelCount;
			srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
			break;
		}
		case Image::Type::TYPE_2D:
		{
			if (resourceDesc.SampleDesc.Count > 1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
				break;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;
				srvDesc.Texture2D.MipLevels = m_CI.subresourceRange.mipLevelCount;
				srvDesc.Texture2D.PlaneSlice = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				break;
			}
		}
		case  Image::Type::TYPE_3D:
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;
			srvDesc.Texture3D.MipLevels = m_CI.subresourceRange.mipLevelCount;
			srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
			break;
		}
		case Image::Type::TYPE_CUBE:
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;
			srvDesc.TextureCube.MipLevels = m_CI.subresourceRange.mipLevelCount;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		case Image::Type::TYPE_1D_ARRAY:
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			srvDesc.Texture1DArray.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;
			srvDesc.Texture1DArray.MipLevels = m_CI.subresourceRange.mipLevelCount;
			srvDesc.Texture1DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
			srvDesc.Texture1DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
			srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
			break;
		}
		case Image::Type::TYPE_2D_ARRAY:
		{
			if (resourceDesc.SampleDesc.Count > 1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srvDesc.Texture2DMSArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
				srvDesc.Texture2DMSArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
				break;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;
				srvDesc.Texture2DArray.MipLevels = m_CI.subresourceRange.mipLevelCount;
				srvDesc.Texture2DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
				srvDesc.Texture2DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
				srvDesc.Texture2DArray.PlaneSlice = 0;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
				break;
			}
		}
		case Image::Type::TYPE_CUBE_ARRAY:
		{
			srvDesc.TextureCubeArray.MostDetailedMip = m_CI.subresourceRange.baseMipLevel;;
			srvDesc.TextureCubeArray.MipLevels = m_CI.subresourceRange.mipLevelCount;
			srvDesc.TextureCubeArray.First2DArrayFace = 0;
			srvDesc.TextureCubeArray.NumCubes = resourceDesc.DepthOrArraySize / 6;
			srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
		}
		}
		m_Device->CreateShaderResourceView(image, &srvDesc, descriptorHandle);
		descriptorHandle.ptr += srvDescriptorSize;
	}

	//UAV
	UINT uavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (uint32_t i = m_CI.subresourceRange.baseMipLevel; i < m_CI.subresourceRange.mipLevelCount; i++)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = resourceDesc.Format;

		switch (std::dynamic_pointer_cast<Image>(m_CI.pImage)->GetCreateInfo().type)
		{
		case Image::Type::TYPE_1D:
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			uavDesc.Texture1D.MipSlice = i;
			break;
		}
		case Image::Type::TYPE_2D:
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = i;
			uavDesc.Texture2D.PlaneSlice = 0;
			break;
		}
		case Image::Type::TYPE_3D:
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture3D.FirstWSlice = m_CI.subresourceRange.baseArrayLayer;
			uavDesc.Texture3D.WSize = m_CI.subresourceRange.arrayLayerCount;
			uavDesc.Texture3D.MipSlice = i;
		}
		case Image::Type::TYPE_1D_ARRAY:
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			uavDesc.Texture1DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
			uavDesc.Texture1DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
			uavDesc.Texture1DArray.MipSlice = i;
			break;
		}
		case Image::Type::TYPE_CUBE:
		case Image::Type::TYPE_CUBE_ARRAY:
		case Image::Type::TYPE_2D_ARRAY:
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.FirstArraySlice = m_CI.subresourceRange.baseArrayLayer;
			uavDesc.Texture2DArray.ArraySize = m_CI.subresourceRange.arrayLayerCount;
			uavDesc.Texture2DArray.MipSlice = i;
			uavDesc.Texture2DArray.PlaneSlice = 0;
			break;
		}
		}
		m_Device->CreateUnorderedAccessView(image, nullptr, &uavDesc, descriptorHandle);
		descriptorHandle.ptr += uavDescriptorSize;
	}
}

ImageView::~ImageView()
{
}