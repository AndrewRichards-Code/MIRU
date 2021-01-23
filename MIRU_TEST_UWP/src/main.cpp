#include "miru_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../dep/STBI/stb_image.h"

#include "../dep/glm/glm/glm.hpp"
#include "../dep/glm/glm/gtc/matrix_transform.hpp"

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.Graphics.Display.h>

using namespace miru;
using namespace crossplatform;

using namespace glm;

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Windows::Graphics::Display;


class MIRU_TEST_UWP : public implements<MIRU_TEST_UWP, IFrameworkViewSource, IFrameworkView>
{
public:
	bool m_exit = false;
	uint32_t width;
	uint32_t height;
	bool windowResize;
	bool shaderRecompile;
	int var_x, var_y, var_z = 0;

	Ref<Context> context;
	Ref<Swapchain> swapchain;
	Swapchain::CreateInfo swapchainCI;

	Ref<Shader> vertexShader, fragmentShader;

	Ref<CommandPool> cmdPool, cmdCopyPool;
	Ref<CommandBuffer> cmdBuffer, cmdCopyBuffer;

	Ref<Allocator> cpu_alloc_0, gpu_alloc_0;

	Ref<Buffer> c_vb, g_vb, c_ib, g_ib, c_imageBuffer;
	Ref<BufferView> vbv, ibv;

	Ref<Image> image, colourImage, depthImage;
	Ref<ImageView> imageView, colourImageView, depthImageView;
	Ref<Sampler> sampler;

	mat4 proj, view, modl;
	float ubData[2 * sizeof(mat4)];
	Ref<Buffer> ub1, ub2;
	Ref<BufferView> ubViewCam, ubViewMdl;

	Ref<DescriptorPool> descriptorPool;
	Ref<DescriptorSetLayout> setLayout1, setLayout2;
	Ref<DescriptorSet> descriptorSet;

	Ref<RenderPass> renderPass;
	Ref<Pipeline> pipeline;
	Ref<Framebuffer> framebuffer0, framebuffer1;


	IFrameworkView CreateView()
	{
		return *this;
	}

	void Initialize(CoreApplicationView const& applicationView)
	{
		//applicationView.Activated({ this, &MIRU_TEST_UWP::OnActivated });

		//CoreApplication::Suspending({ this, &MIRU_TEST_UWP::OnSuspending });

		//CoreApplication::Resuming({ this, &MIRU_TEST_UWP::OnResuming });
	}

	void Load(hstring const&)
	{
	}

	void Uninitialize()
	{
		context->DeviceWaitIdle();
	}

	void Run()
	{
		CoreWindow::GetForCurrentThread().Activate();

		Fence::CreateInfo fenceCI;
		fenceCI.debugName = "DrawFence";
		fenceCI.device = context->GetDevice();
		fenceCI.signaled = true;
		fenceCI.timeout = UINT64_MAX;
		std::vector<Ref<Fence>>draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
		Semaphore::CreateInfo acquireSemaphoreCI = { "AcquireSeamphore", context->GetDevice() };
		Semaphore::CreateInfo submitSemaphoreCI = { "SubmitSeamphore", context->GetDevice() };
		std::vector<Ref<Semaphore>>acquire = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
		std::vector<Ref<Semaphore>>submit = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };

		uint32_t frameIndex = 0;
		uint32_t frameCount = 0;
		float r = 1.00f;
		float g = 0.00f;
		float b = 0.00f;
		float increment = 1.0f / 60.0f;

		while (!m_exit)
		{
			{
				if (r > b && b < increment)
				{
					b = 0.0f;
					r -= increment;
					g += increment;
				}
				if (g > r && r < increment)
				{
					r = 0.0f;
					g -= increment;
					b += increment;
				}
				if (b > g && g < increment)
				{
					g = 0.0f;
					b -= increment;
					r += increment;
				}

				draws[frameIndex]->Wait();

				cmdBuffer->Reset(frameIndex, false);
				cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
				cmdBuffer->BeginRenderPass(frameIndex, frameIndex == 0 ? framebuffer0 : framebuffer1, { {r, g, b, 1.0f}, {0.0f, 0} });
				cmdBuffer->BindPipeline(frameIndex, pipeline);
				cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet }, pipeline);
				cmdBuffer->BindVertexBuffers(frameIndex, { vbv });
				cmdBuffer->BindIndexBuffer(frameIndex, ibv);
				cmdBuffer->DrawIndexed(frameIndex, 36);
				cmdBuffer->EndRenderPass(frameIndex);
				cmdBuffer->End(frameIndex);

				proj = perspectiveFov(radians(90.0f), float(width), float(height), 0.1f, 100.0f);
				if (GraphicsAPI::IsVulkan())
					proj[1][1] *= -1;
				modl = translate(glm::mat4(1.0f), { 0.0f, 0.0f, -1.5f })
					//* translate(mat4(1.0f), { float(var_x)/10.0f, float(var_y)/10.0f, float(var_z)/10.0f})
					* rotate(glm::mat4(1.0f), radians((float)var_x /** 5.0f*/), { 0, 1, 0 })
					* rotate(glm::mat4(1.0f), radians((float)var_y /** 5.0f*/), { 1, 0, 0 })
					* rotate(glm::mat4(1.0f), radians((float)var_z /** 5.0f*/), { 0, 0, 1 });
				proj = transpose(proj);
				modl = transpose(modl);

				memcpy(ubData + 0 * 16, &proj[0][0], sizeof(mat4));
				memcpy(ubData + 1 * 16, &view[0][0], sizeof(mat4));

				cpu_alloc_0->SubmitData(ub1->GetAllocation(), 2 * sizeof(mat4), ubData);
				cpu_alloc_0->SubmitData(ub2->GetAllocation(), sizeof(mat4), (void*)&modl[0][0]);

				var_x++;
				var_y = 20;
			}
			cmdBuffer->Present({ 0, 1 }, swapchain, draws, acquire, submit, windowResize);
			frameIndex = (frameIndex + 1) % swapchainCI.swapchainCount;
			frameCount++;

			CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		}
	}

	void SetWindow(const CoreWindow& window)
	{
		auto navigation = SystemNavigationManager::GetForCurrentView();
		// UWP on Xbox One triggers a back request whenever the B button is pressed
		// which can result in the app being suspended if unhandled
		navigation.BackRequested([](const winrt::Windows::Foundation::IInspectable&, const BackRequestedEventArgs& args)
			{
				args.Handled(true);
			});

		window.Closed([this](auto&&, auto&&) { m_exit = true; });

		float DPI = DisplayInformation::GetForCurrentView().LogicalDpi();

		auto ConvertDipsToPixels = [&](float dips) -> uint32_t
		{
			return int(dips * DPI / 96.f + 0.5f);
		};
		width = ConvertDipsToPixels(window.Bounds().Width);
		height = ConvertDipsToPixels(window.Bounds().Height);

		GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
		GraphicsAPI::AllowSetName();
		GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::NONE);

		Context::CreateInfo contextCI;
		contextCI.api_version_major = 12;
		contextCI.api_version_minor = 1;
		contextCI.applicationName = "MIRU_TEST_UWP";
		contextCI.instanceLayers = {};
		contextCI.instanceExtensions = {};
		contextCI.deviceLayers = {};
		contextCI.deviceExtensions = {};
		contextCI.deviceDebugName = "GPU Device";
		context = Context::Create(&contextCI);

		swapchainCI.debugName = "Swapchain";
		swapchainCI.pContext = context;
		swapchainCI.pWindow = winrt::get_abi(window);
		swapchainCI.width = width;
		swapchainCI.height = height;
		swapchainCI.swapchainCount = 2;
		swapchainCI.vSync = true;
		swapchain = Swapchain::Create(&swapchainCI);
		width = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().pImage->GetCreateInfo().width;
		height = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().pImage->GetCreateInfo().height;

		Shader::CreateInfo shaderCI;
		shaderCI.debugName = "Basic: Vertex Shader Module";
		shaderCI.device = context->GetDevice();
		shaderCI.stage = Shader::StageBit::VERTEX_BIT;
		shaderCI.entryPoint = "vs_main";
		shaderCI.binaryFilepath = "res/bin/basic_vert_vs_main.cso";
		shaderCI.binaryCode = {};
		shaderCI.recompileArguments = {};
		vertexShader = Shader::Create(&shaderCI);

		shaderCI.debugName = "Basic: Fragment Shader Module";
		shaderCI.stage = Shader::StageBit::PIXEL_BIT;
		shaderCI.entryPoint = "ps_main";
		shaderCI.binaryFilepath = "res/bin/basic_frag_ps_main.cso";
		fragmentShader = Shader::Create(&shaderCI);

		CommandPool::CreateInfo cmdPoolCI;
		cmdPoolCI.debugName = "CmdPool";
		cmdPoolCI.pContext = context;
		cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
		cmdPool = CommandPool::Create(&cmdPoolCI);
		cmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
		cmdCopyPool = CommandPool::Create(&cmdPoolCI);

		CommandBuffer::CreateInfo cmdBufferCI, cmdCopyBufferCI;
		cmdBufferCI.debugName = "CmdBuffer";
		cmdBufferCI.pCommandPool = cmdPool;
		cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmdBufferCI.commandBufferCount = 3;
		cmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
		cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
		cmdCopyBufferCI.debugName = "CmdCopyBuffer";
		cmdCopyBufferCI.pCommandPool = cmdCopyPool;
		cmdCopyBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmdCopyBufferCI.commandBufferCount = 1;
		cmdCopyBufferCI.allocateNewCommandPoolPerBuffer = false;
		cmdCopyBuffer = CommandBuffer::Create(&cmdCopyBufferCI);

		Allocator::CreateInfo allocCI;
		allocCI.debugName = "CPU_ALLOC_0";
		allocCI.pContext = context;
		allocCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
		allocCI.properties = Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT;
		cpu_alloc_0 = Allocator::Create(&allocCI);
		allocCI.debugName = "GPU_ALLOC_0";
		allocCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
		gpu_alloc_0 = Allocator::Create(&allocCI);

		float vertices[32] =
		{
			-0.5f, -0.5f, -0.5f, 1.0f,
			+0.5f, -0.5f, -0.5f, 1.0f,
			+0.5f, +0.5f, -0.5f, 1.0f,
			-0.5f, +0.5f, -0.5f, 1.0f,
			-0.5f, -0.5f, +0.5f, 1.0f,
			+0.5f, -0.5f, +0.5f, 1.0f,
			+0.5f, +0.5f, +0.5f, 1.0f,
			-0.5f, +0.5f, +0.5f, 1.0f,
		};
		uint32_t indices[36] = {
			0, 1, 2, 2, 3, 0,
			1, 5, 6, 6, 2, 1,
			5, 4, 7, 7, 6, 5,
			4, 0, 3, 3, 7, 4,
			3, 2, 6, 6, 7, 3,
			4, 5, 1, 1, 0, 4
		};

		int img_width;
		int img_height;
		int bpp;
		uint8_t* imageData = stbi_load("logo.png", &img_width, &img_height, &bpp, 4);

		Buffer::CreateInfo verticesBufferCI;
		verticesBufferCI.debugName = "Vertices Buffer";
		verticesBufferCI.device = context->GetDevice();
		verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
		verticesBufferCI.size = sizeof(vertices);
		verticesBufferCI.data = vertices;
		verticesBufferCI.pAllocator = cpu_alloc_0;
		c_vb = Buffer::Create(&verticesBufferCI);
		verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT;
		verticesBufferCI.data = nullptr;
		verticesBufferCI.pAllocator = gpu_alloc_0;
		g_vb = Buffer::Create(&verticesBufferCI);

		Buffer::CreateInfo indicesBufferCI;
		indicesBufferCI.debugName = "Indices Buffer";
		indicesBufferCI.device = context->GetDevice();
		indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
		indicesBufferCI.size = sizeof(indices);
		indicesBufferCI.data = indices;
		indicesBufferCI.pAllocator = cpu_alloc_0;
		c_ib = Buffer::Create(&indicesBufferCI);
		indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
		indicesBufferCI.data = nullptr;
		indicesBufferCI.pAllocator = gpu_alloc_0;
		g_ib = Buffer::Create(&indicesBufferCI);

		Buffer::CreateInfo imageBufferCI;
		imageBufferCI.debugName = "MIRU logo upload buffer";
		imageBufferCI.device = context->GetDevice();
		imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
		imageBufferCI.size = img_width * img_height * 4;
		imageBufferCI.data = imageData;
		imageBufferCI.pAllocator = cpu_alloc_0;
		c_imageBuffer = Buffer::Create(&imageBufferCI);
		Image::CreateInfo imageCI;
		imageCI.debugName = "MIRU logo Image";
		imageCI.device = context->GetDevice();
		imageCI.type = Image::Type::TYPE_CUBE;
		imageCI.format = Image::Format::R8G8B8A8_UNORM;
		imageCI.width = img_width;
		imageCI.height = img_height;
		imageCI.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 6;
		imageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		imageCI.usage = Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::SAMPLED_BIT;
		imageCI.layout = Image::Layout::UNKNOWN;
		imageCI.size = img_width * img_height * 4;
		imageCI.data = nullptr;
		imageCI.pAllocator = gpu_alloc_0;
		image = Image::Create(&imageCI);

		Semaphore::CreateInfo transSemaphoreCI = { "TransferSemaphore", context->GetDevice() };
		Ref<Semaphore> transfer = Semaphore::Create(&transSemaphoreCI);
		{
			cmdCopyBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

			cmdCopyBuffer->CopyBuffer(0, c_vb, g_vb, { { 0, 0, sizeof(vertices) } });
			cmdCopyBuffer->CopyBuffer(0, c_ib, g_ib, { { 0, 0, sizeof(indices) } });

			Barrier::CreateInfo bCI;
			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
			bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
			bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			bCI.pImage = image;
			bCI.oldLayout = Image::Layout::UNKNOWN;
			bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
			Ref<Barrier> b = Barrier::Create(&bCI);
			cmdCopyBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b });
			cmdCopyBuffer->CopyBufferToImage(0, c_imageBuffer, image, Image::Layout::TRANSFER_DST_OPTIMAL, {
				{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
				{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 1, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
				{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 2, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
				{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 3, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
				{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 4, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}},
				{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 5, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}}
				});

			cmdCopyBuffer->End(0);
		}
		cmdCopyBuffer->Submit({ 0 }, {}, {}, { transfer }, nullptr);
		{
			cmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

			Barrier::CreateInfo bCI;
			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
			bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
			bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
			bCI.pImage = image;
			bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
			bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
			Ref<Barrier> b = Barrier::Create(&bCI);
			cmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, { b });

			cmdBuffer->End(2);
		}
		cmdBuffer->Submit({ 2 }, { transfer }, { PipelineStageBit::TRANSFER_BIT }, {}, nullptr);

		BufferView::CreateInfo vbViewCI;
		vbViewCI.debugName = "VerticesBufferView";
		vbViewCI.device = context->GetDevice();
		vbViewCI.type = BufferView::Type::VERTEX;
		vbViewCI.pBuffer = g_vb;
		vbViewCI.offset = 0;
		vbViewCI.size = sizeof(vertices);
		vbViewCI.stride = 4 * sizeof(float);
		vbv = BufferView::Create(&vbViewCI);

		BufferView::CreateInfo ibViewCI;
		ibViewCI.debugName = "IndicesBufferView";
		ibViewCI.device = context->GetDevice();
		ibViewCI.type = BufferView::Type::INDEX;
		ibViewCI.pBuffer = g_ib;
		ibViewCI.offset = 0;
		ibViewCI.size = sizeof(indices);
		ibViewCI.stride = sizeof(uint32_t);
		ibv = BufferView::Create(&ibViewCI);

		ImageView::CreateInfo imageViewCI;
		imageViewCI.debugName = "MIRU logo ImageView";
		imageViewCI.device = context->GetDevice();
		imageViewCI.pImage = image;
		imageViewCI.viewType = Image::Type::TYPE_CUBE;
		imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
		imageView = ImageView::Create(&imageViewCI);

		Sampler::CreateInfo samplerCI;
		samplerCI.debugName = "Default Sampler";
		samplerCI.device = context->GetDevice();
		samplerCI.magFilter = Sampler::Filter::NEAREST;
		samplerCI.minFilter = Sampler::Filter::NEAREST;
		samplerCI.mipmapMode = Sampler::MipmapMode::NEAREST;
		samplerCI.addressModeU = Sampler::AddressMode::REPEAT;
		samplerCI.addressModeV = Sampler::AddressMode::REPEAT;
		samplerCI.addressModeW = Sampler::AddressMode::REPEAT;
		samplerCI.mipLodBias = 1;
		samplerCI.anisotropyEnable = false;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareEnable = false;
		samplerCI.compareOp = CompareOp::NEVER;
		samplerCI.minLod = 0;
		samplerCI.maxLod = 1;
		samplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
		samplerCI.unnormalisedCoordinates = false;
		Ref<Sampler> sampler = Sampler::Create(&samplerCI);

		proj = identity<mat4>();
		view = identity<mat4>();
		modl = identity<mat4>();

		float ubData[2 * sizeof(mat4)];
		memcpy(ubData + 0 * 16, &proj[0][0], sizeof(mat4));
		memcpy(ubData + 1 * 16, &view[0][0], sizeof(mat4));

		Buffer::CreateInfo ubCI;
		ubCI.debugName = "Camera UB";
		ubCI.device = context->GetDevice();
		ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
		ubCI.size = 2 * sizeof(mat4);
		ubCI.data = ubData;
		ubCI.pAllocator = cpu_alloc_0;
		ub1 = Buffer::Create(&ubCI);
		ubCI.debugName = "Model UB";
		ubCI.device = context->GetDevice();
		ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
		ubCI.size = sizeof(mat4);
		ubCI.data = &modl[0][0];
		ubCI.pAllocator = cpu_alloc_0;
		ub2 = Buffer::Create(&ubCI);

		BufferView::CreateInfo ubViewCamCI;
		ubViewCamCI.debugName = "Camera UBView";
		ubViewCamCI.device = context->GetDevice();
		ubViewCamCI.type = BufferView::Type::UNIFORM;
		ubViewCamCI.pBuffer = ub1;
		ubViewCamCI.offset = 0;
		ubViewCamCI.size = 2 * sizeof(mat4);
		ubViewCamCI.stride = 0;
		ubViewCam = BufferView::Create(&ubViewCamCI);
		BufferView::CreateInfo ubViewMdlCI;
		ubViewMdlCI.debugName = "Model UBView";
		ubViewMdlCI.device = context->GetDevice();
		ubViewMdlCI.type = BufferView::Type::UNIFORM;
		ubViewMdlCI.pBuffer = ub2;
		ubViewMdlCI.offset = 0;
		ubViewMdlCI.size = sizeof(mat4);
		ubViewMdlCI.stride = 0;
		ubViewMdl = BufferView::Create(&ubViewMdlCI);

		Image::CreateInfo colourCI;
		colourCI.debugName = "Colour Image MSAA";
		colourCI.device = context->GetDevice();
		colourCI.type = Image::Type::TYPE_2D;;
		colourCI.format = swapchain->m_SwapchainImages[0]->GetCreateInfo().format;
		colourCI.width = width;
		colourCI.height = height;
		colourCI.depth = 1;
		colourCI.mipLevels = 1;
		colourCI.arrayLayers = 1;
		colourCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_8_BIT;
		colourCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT;
		colourCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::COLOUR_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		colourCI.size = 0;
		colourCI.data = nullptr;
		colourCI.pAllocator = gpu_alloc_0;
		colourImage = Image::Create(&colourCI);

		ImageView::CreateInfo colourImageViewCI;
		colourImageViewCI.debugName = "Colour ImageView";
		colourImageViewCI.device = context->GetDevice();
		colourImageViewCI.pImage = colourImage;
		colourImageViewCI.viewType = Image::Type::TYPE_2D;
		colourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		colourImageView = ImageView::Create(&colourImageViewCI);

		Image::CreateInfo depthCI;
		depthCI.debugName = "Depth Image";
		depthCI.device = context->GetDevice();
		depthCI.type = Image::Type::TYPE_2D;
		depthCI.format = Image::Format::D32_SFLOAT;
		depthCI.width = width;
		depthCI.height = height;
		depthCI.depth = 1;
		depthCI.mipLevels = 1;
		depthCI.arrayLayers = 1;
		depthCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_8_BIT;
		depthCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
		depthCI.layout = GraphicsAPI::IsD3D12() ? Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
		depthCI.size = 0;
		depthCI.data = nullptr;
		depthCI.pAllocator = gpu_alloc_0;
		depthImage = Image::Create(&depthCI);

		ImageView::CreateInfo depthImageViewCI;
		depthImageViewCI.debugName = "Depth ImageView";
		depthImageViewCI.device = context->GetDevice();
		depthImageViewCI.pImage = depthImage;
		depthImageViewCI.viewType = Image::Type::TYPE_2D;
		depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
		depthImageView = ImageView::Create(&depthImageViewCI);

		DescriptorPool::CreateInfo descriptorPoolCI;
		descriptorPoolCI.debugName = "Basic: Descriptor Pool";
		descriptorPoolCI.device = context->GetDevice();
		descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1},  {DescriptorType::UNIFORM_BUFFER, 2} };
		descriptorPoolCI.maxSets = 2;
		descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
		DescriptorSetLayout::CreateInfo setLayoutCI;
		setLayoutCI.debugName = "Basic: DescSetLayout1";
		setLayoutCI.device = context->GetDevice();
		setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT } };
		setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
		setLayoutCI.debugName = "Basic: DescSetLayout2";
		setLayoutCI.descriptorSetLayoutBinding = {
			{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT },
			{1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT }
		};
		setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
		DescriptorSet::CreateInfo descriptorSetCI;
		descriptorSetCI.debugName = "Basic: Descriptor Set";
		descriptorSetCI.pDescriptorPool = descriptorPool;
		descriptorSetCI.pDescriptorSetLayouts = { setLayout1, setLayout2 };
		descriptorSet = DescriptorSet::Create(&descriptorSetCI);
		descriptorSet->AddBuffer(0, 0, { { ubViewCam } });
		descriptorSet->AddBuffer(1, 0, { { ubViewMdl } });
		descriptorSet->AddImage(1, 1, { { sampler, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		descriptorSet->Update();

		RenderPass::CreateInfo renderPassCI;
		renderPassCI.debugName = "Basic: RenderPass";
		renderPassCI.device = context->GetDevice();
		renderPassCI.attachments = {
			{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
			Image::SampleCountBit::SAMPLE_COUNT_8_BIT,
			RenderPass::AttachmentLoadOp::CLEAR,
			RenderPass::AttachmentStoreOp::STORE,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			GraphicsAPI::IsD3D12() ? Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
			Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
			},
			{depthImage->GetCreateInfo().format,
			Image::SampleCountBit::SAMPLE_COUNT_8_BIT,
			RenderPass::AttachmentLoadOp::CLEAR,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			Image::Layout::UNKNOWN,
			Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			},
			{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
			Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::STORE,
			RenderPass::AttachmentLoadOp::DONT_CARE,
			RenderPass::AttachmentStoreOp::DONT_CARE,
			Image::Layout::UNKNOWN,
			Image::Layout::PRESENT_SRC
			}
		};
		renderPassCI.subpassDescriptions = {
			{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {{2, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {} }
		};
		renderPassCI.subpassDependencies = {
			{MIRU_SUBPASS_EXTERNAL, 0,
			PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
			(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
		};
		renderPass = RenderPass::Create(&renderPassCI);

		Pipeline::CreateInfo pCI;
		pCI.debugName = "Basic";
		pCI.device = context->GetDevice();
		pCI.type = PipelineType::GRAPHICS;
		pCI.shaders = { vertexShader, fragmentShader };
		pCI.vertexInputState.vertexInputBindingDescriptions = { {0, sizeof(vertices) / 8, VertexInputRate::VERTEX} };
		pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0, "POSITION"} };
		pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
		pCI.tessellationState = {};
		pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
		pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
		pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
		pCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_8_BIT, false, 1.0f, false, false };
		pCI.depthStencilState = { true, true, CompareOp::GREATER, false, false, {}, {}, 0.0f, 1.0f };
		pCI.colourBlendState.logicOpEnable = false;
		pCI.colourBlendState.logicOp = LogicOp::COPY;
		pCI.colourBlendState.attachments = { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
												BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } };
		pCI.colourBlendState.blendConstants[0] = 0.0f;
		pCI.colourBlendState.blendConstants[1] = 0.0f;
		pCI.colourBlendState.blendConstants[2] = 0.0f;
		pCI.colourBlendState.blendConstants[3] = 0.0f;
		pCI.dynamicStates = {};
		pCI.layout = { {setLayout1, setLayout2 }, {} };
		pCI.renderPass = renderPass;
		pCI.subpassIndex = 0;
		pipeline = Pipeline::Create(&pCI);

		Framebuffer::CreateInfo framebufferCI_0, framebufferCI_1;
		framebufferCI_0.debugName = "Framebuffer0";
		framebufferCI_0.device = context->GetDevice();
		framebufferCI_0.renderPass = renderPass;
		framebufferCI_0.attachments = { colourImageView, depthImageView, swapchain->m_SwapchainImageViews[0] };
		framebufferCI_0.width = width;
		framebufferCI_0.height = height;
		framebufferCI_0.layers = 1;
		framebuffer0 = Framebuffer::Create(&framebufferCI_0);
		framebufferCI_1.debugName = "Framebuffer1";
		framebufferCI_1.device = context->GetDevice();
		framebufferCI_1.renderPass = renderPass;
		framebufferCI_1.attachments = { colourImageView, depthImageView, swapchain->m_SwapchainImageViews[1] };
		framebufferCI_1.width = width;
		framebufferCI_1.height = height;
		framebufferCI_1.layers = 1;
		framebuffer1 = Framebuffer::Create(&framebufferCI_1);
	}

	protected:
	// Event handlers
	/*void OnActivated(CoreApplicationView const& applicationView, IActivatedEventArgs const& args)
	{
		CoreWindow::GetForCurrentThread().Activate();
	}

	void OnSuspending(IInspectable const& sender, SuspendingEventArgs const& args)
	{
	}

	void OnResuming(IInspectable const& sender, IInspectable const& args)
	{
	}*/
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	CoreApplication::Run(make<MIRU_TEST_UWP>());
}