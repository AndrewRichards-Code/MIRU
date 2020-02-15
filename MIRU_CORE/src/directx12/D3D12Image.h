#pragma once
#include "crossplatform/Image.h"

namespace miru
{
namespace d3d12
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
		D3D12_RESOURCE_DIMENSION ToD3D12ImageType(Image::Type type) const;

	public:
		static DXGI_FORMAT ToD3D12ImageFormat(Image::Format format);
		static D3D12_RESOURCE_STATES ToD3D12ImageLayout(Image::Layout layout);

		//Members
	public:
		ID3D12Device* m_Device;

		ID3D12Resource* m_Image;
		D3D12_RESOURCE_DESC m_ResourceDesc;
		D3D12_RESOURCE_STATES m_CurrentResourceState;
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
		ID3D12Device* m_Device;

		D3D12_RENDER_TARGET_VIEW_DESC m_RTVDesc = {};
		D3D12_DEPTH_STENCIL_VIEW_DESC m_DSVDesc = {};
		D3D12_SHADER_RESOURCE_VIEW_DESC m_SRVDesc = {};
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_UAVDesc = {};

		ID3D12Resource* m_ImageView = nullptr;
	};

	class Sampler final : public crossplatform::Sampler
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
	};
}
}