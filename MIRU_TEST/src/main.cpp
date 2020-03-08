#include "miru_core.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../dep/STBI/stb_image.h"

using namespace miru;
using namespace crossplatform;

HWND window;
bool g_WindowQuit = false;
uint32_t width = 800;
uint32_t height = 600;
bool windowResize;
bool shaderRecompile;
LRESULT CALLBACK WindProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY || msg == WM_CLOSE)
	{
		PostQuitMessage(0);
		g_WindowQuit = true;
		return 0;
	}
	if (msg == WM_SIZE)
	{
		width = LOWORD(lparam);
		height = HIWORD(lparam);
	}
	if (msg == WM_EXITSIZEMOVE)
	{
		windowResize = true;
	}
	if (msg == WM_KEYDOWN && wparam == 0x52) //R
	{
		shaderRecompile = true;
	}


	return DefWindowProc(handle, msg, wparam, lparam);
}
void WindowUpdate()
{
	MSG msg = { 0 };
	if (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int main()
{
	GraphicsAPI api;
	api.LoadRenderDoc();
	api.AllowSetName();
	//api.SetAPI(GraphicsAPI::API::D3D12);
	api.SetAPI(GraphicsAPI::API::VULKAN);

	Context::CreateInfo contextCI;
	contextCI.api_version_major = api.IsD3D12() ? 11 : 1;
	contextCI.api_version_minor = 1;
	contextCI.applicationName = "MIRU_TEST";
	contextCI.instanceLayers = { "VK_LAYER_LUNARG_standard_validation" };
	contextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	contextCI.deviceLayers = { "VK_LAYER_LUNARG_standard_validation" };
	contextCI.deviceExtensions = { "VK_KHR_swapchain" };
	contextCI.deviceDebugName = "GPU Device";
	Ref<Context> context = Context::Create(&contextCI);

	//Creates the windows
	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = contextCI.applicationName;
	RegisterClass(&wc);

	window = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, 100, 100, width, height, 0, 0, 0, 0);
	ShowWindow(window, SW_SHOW);

	Swapchain::CreateInfo swapchainCI;
	swapchainCI.debugName = "Swapchain";
	swapchainCI.pContext = context;
	swapchainCI.pWindow = window;
	swapchainCI.width = width;
	swapchainCI.height = height;
	swapchainCI.swapchainCount = 2;
	swapchainCI.vSync = true;
	Ref<Swapchain> swapchain = Swapchain::Create(&swapchainCI);
	width = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().pImage->GetCreateInfo().width;
	height = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().pImage->GetCreateInfo().height;

	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "Basic_Vertex";
	shaderCI.device = context->GetDevice();
	shaderCI.stage = Shader::StageBit::VERTEX_BIT;
	shaderCI.filepath = "res/bin/basic.vert.spv";
	shaderCI.entryPoint = "main";
	Ref<Shader> vertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Basic_Fragment";
	shaderCI.stage = Shader::StageBit::PIXEL_BIT;
	shaderCI.filepath = "res/bin/basic.frag.spv";
	Ref<Shader> fragmentShader = Shader::Create(&shaderCI);

	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CmdPool";
	cmdPoolCI.pContext = context;
	cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	cmdPoolCI.queueFamilyIndex = 0;
	Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);
	cmdPoolCI.queueFamilyIndex = 2;
	Ref<CommandPool> cmdCopyPool = CommandPool::Create(&cmdPoolCI);

	CommandBuffer::CreateInfo cmdBufferCI, cmdCopyBufferCI;
	cmdBufferCI.debugName = "CmdBuffer";
	cmdBufferCI.pCommandPool = cmdPool;
	cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdBufferCI.commandBufferCount = 3;
	Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
	cmdCopyBufferCI.debugName = "CmdCopyBuffer";
	cmdCopyBufferCI.pCommandPool = cmdCopyPool;
	cmdCopyBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdCopyBufferCI.commandBufferCount = 1;
	Ref<CommandBuffer> cmdCopyBuffer = CommandBuffer::Create(&cmdCopyBufferCI);

	MemoryBlock::CreateInfo mbCI;
	mbCI.debugName = "CPU_MB_0";
	mbCI.pContext = context;
	mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_8MB;
	mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
	Ref<MemoryBlock> cpu_mb_0 = MemoryBlock::Create(&mbCI);
	mbCI.debugName = "GPU_MB_0";
	mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT ;
	Ref<MemoryBlock> gpu_mb_0 = MemoryBlock::Create(&mbCI);

	float vertices[16] =
	{
		-0.5f, -0.5f, 0.0f, 1.0f,
		+0.5f, -0.5f, 0.0f, 1.0f,
		+0.5f, +0.5f, 0.0f, 1.0f,
		-0.5f, +0.5f, 0.0f, 1.0f
	};
	uint32_t indices[6] = { 0,1,2,2,3,0 };

	int img_width;
	int img_height; 
	int bpp;
	uint8_t* imageData = stbi_load("../logo.png", &img_width, &img_height, &bpp, 4);

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices";
	verticesBufferCI.device = context->GetDevice();
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	verticesBufferCI.size = sizeof(vertices);
	verticesBufferCI.data = vertices;
	verticesBufferCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> c_vb = Buffer::Create(&verticesBufferCI);
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::VERTEX;
	verticesBufferCI.data = nullptr;
	verticesBufferCI.pMemoryBlock = gpu_mb_0;
	Ref<Buffer> g_vb = Buffer::Create(&verticesBufferCI);

	Buffer::CreateInfo indicesBufferCI;
	indicesBufferCI.debugName = "Indices";
	indicesBufferCI.device = context->GetDevice();
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> c_ib = Buffer::Create(&indicesBufferCI);
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::INDEX;
	indicesBufferCI.data = nullptr;
	indicesBufferCI.pMemoryBlock = gpu_mb_0;
	Ref<Buffer> g_ib = Buffer::Create(&indicesBufferCI);

	Buffer::CreateInfo imageBufferCI;
	imageBufferCI.debugName = "Image";
	imageBufferCI.device = context->GetDevice();
	imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	imageBufferCI.size = img_width * img_height * 4;
	imageBufferCI.data = imageData;
	imageBufferCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> c_imageBuffer = Buffer::Create(&imageBufferCI);
	Image::CreateInfo imageCI;
	imageCI.debugName = "Image";;
	imageCI.device = context->GetDevice();
	imageCI.type = Image::Type::TYPE_2D;
	imageCI.format = Image::Format::R8G8B8A8_UNORM;
	imageCI.width = img_width;
	imageCI.height = img_height;
	imageCI.depth = 1;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	imageCI.usage = Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::SAMPLED_BIT;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = img_width * img_height * 4;
	imageCI.data = nullptr;
	imageCI.pMemoryBlock = gpu_mb_0;
	Ref<Image> image = Image::Create(&imageCI);

	Semaphore::CreateInfo transSemaphoreCI = { "Transfer", context->GetDevice() };
	Ref<Semaphore> transfer = Semaphore::Create(&transSemaphoreCI);
	{
		cmdCopyBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		
		cmdCopyBuffer->CopyBuffer(0, c_vb, g_vb, { { 0, 0, sizeof(vertices) } });
		cmdCopyBuffer->CopyBuffer(0, c_ib, g_ib, { { 0, 0, sizeof(indices) } });

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE;
		bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		bCI.pImage = image;
		bCI.oldLayout = Image::Layout::UNKNOWN;
		bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.subresoureRange = { Image::AspectBit::COLOR_BIT, 0, 1, 0, 1 };
		Ref<Barrier> b = Barrier::Create(&bCI);
		cmdCopyBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, { b });
		cmdCopyBuffer->CopyBufferToImage(0, c_imageBuffer, image, Image::Layout::TRANSFER_DST_OPTIMAL, { {0, 0, 0, {Image::AspectBit::COLOR_BIT, 0, 0, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}} });
		
		cmdCopyBuffer->End(0);
	}
	cmdCopyBuffer->Submit({ 0 }, {}, { transfer }, PipelineStageBit::TRANSFER_BIT, nullptr);
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
		bCI.subresoureRange = { Image::AspectBit::COLOR_BIT, 0, 1, 0, 1 };
		Ref<Barrier> b = Barrier::Create(&bCI);
		cmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, { b });

		cmdBuffer->End(2);
	}
	cmdBuffer->Submit({ 2 }, { transfer }, {}, PipelineStageBit::TRANSFER_BIT, nullptr);

	BufferView::CreateInfo vbViewCI;
	vbViewCI.debugName = "VerticesBufferView";
	vbViewCI.device = context->GetDevice();
	vbViewCI.type = BufferView::Type::VERTEX;
	vbViewCI.pBuffer = g_vb;
	vbViewCI.offset = 0;
	vbViewCI.size = sizeof(vertices);
	vbViewCI.stride = 4 * sizeof(float);
	Ref<BufferView> vbv = BufferView::Create(&vbViewCI);

	BufferView::CreateInfo ibViewCI;
	ibViewCI.debugName = "VerticesBufferView";
	ibViewCI.device = context->GetDevice();
	ibViewCI.type = BufferView::Type::INDEX;
	ibViewCI.pBuffer = g_ib;
	ibViewCI.offset = 0;
	ibViewCI.size = sizeof(indices);
	ibViewCI.stride = sizeof(uint32_t);
	Ref<BufferView> ibv = BufferView::Create(&ibViewCI);

	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "ImageView";
	imageViewCI.device = context->GetDevice();
	imageViewCI.pImage = image;
	imageViewCI.subresourceRange = { Image::AspectBit::COLOR_BIT, 0, 1, 0, 1 };
	Ref<ImageView> imageView = ImageView::Create(&imageViewCI);

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

	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "Image Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1} };
	descriptorPoolCI.maxSets = 1;
	Ref<DescriptorPool> descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "Basic Shader DescSetLayout";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::COMBINED_IMAGE_SAMPLER,  1, Shader::StageBit::FRAGMENT_BIT } };
	Ref<DescriptorSetLayout> setLayout = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetCI;
	descriptorSetCI.debugName = "Image Descriptor Set";
	descriptorSetCI.pDescriptorPool = descriptorPool;
	descriptorSetCI.pDescriptorSetLayouts = {setLayout};
	Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(&descriptorSetCI);
	descriptorSet->AddImage(0, 0, { { sampler, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descriptorSet->Update();

	RenderPass::CreateInfo renderPassCI;
	renderPassCI.debugName = "Basic";
	renderPassCI.device = context->GetDevice();
	renderPassCI.attachments = {
		{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::STORE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		api.IsD3D12()?Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
		Image::Layout::PRESENT_SRC
		}
	};
	renderPassCI.subpassDescriptions = {
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOR_ATTACHMENT_OPTIMAL}}, {},{},{} }
	};
	renderPassCI.subpassDependencies = {
		{MIRU_SUBPASS_EXTERNAL, 0,
		PipelineStageBit::COLOR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOR_ATTACHMENT_WRITE_BIT}
	};
	Ref<RenderPass> renderPass = RenderPass::Create(&renderPassCI);

	Pipeline::CreateInfo pCI;
	pCI.debugName = "Basic";
	pCI.device = context->GetDevice();
	pCI.type = PipelineType::GRAPHICS;
	pCI.shaders = { vertexShader, fragmentShader };
	pCI.vertexInputState.vertexInputBindingDescriptions = { {0, 16, VertexInputRate::VERTEX} };
	pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0, "POSITION"} };
	pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
	pCI.tessellationState = {};
	pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
	pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
	pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false };
	pCI.depthStencilState = { true, true, CompareOp::LESS, false, false, {}, {}, 0.0f, 1.0f };
	pCI.colourBlendState.logicOpEnable = false;
	pCI.colourBlendState.logicOp = LogicOp::COPY;
	pCI.colourBlendState.attachments = { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
											BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } };
	pCI.colourBlendState.blendConstants[0] = 0.0f;
	pCI.colourBlendState.blendConstants[1] = 0.0f;
	pCI.colourBlendState.blendConstants[2] = 0.0f;
	pCI.colourBlendState.blendConstants[3] = 0.0f;
	pCI.dynamicStates = {};
	pCI.layout = { {setLayout}, {} };
	pCI.renderPass = renderPass;
	pCI.subpassIndex = 0;
	Ref<Pipeline> pipeline = Pipeline::Create(&pCI);

	Framebuffer::CreateInfo framebufferCI_0, framebufferCI_1;
	framebufferCI_0.debugName = "Framebuffer0";
	framebufferCI_0.device = context->GetDevice();
	framebufferCI_0.renderPass = renderPass;
	framebufferCI_0.attachments = { swapchain->m_SwapchainImageViews[0] };
	framebufferCI_0.width = width;
	framebufferCI_0.height = height;
	framebufferCI_0.layers = 1;
	Ref<Framebuffer> framebuffer0 = Framebuffer::Create(&framebufferCI_0);
	framebufferCI_1.debugName = "Framebuffer1";
	framebufferCI_1.device = context->GetDevice();
	framebufferCI_1.renderPass = renderPass;
	framebufferCI_1.attachments = { swapchain->m_SwapchainImageViews[1] };
	framebufferCI_1.width = width;
	framebufferCI_1.height = height;
	framebufferCI_1.layers = 1;
	Ref<Framebuffer> framebuffer1 = Framebuffer::Create(&framebufferCI_1);

	auto RecordPresentCmdBuffers = [&]()
	{	
		for (uint32_t i = 0; i < cmdBuffer->GetCreateInfo().commandBufferCount - 1; i++)
		{
			cmdBuffer->Begin(i, CommandBuffer::UsageBit::SIMULTANEOUS);
			cmdBuffer->BeginRenderPass(i, i == 0 ? framebuffer0 : framebuffer1, { {0.0f, 0.0f, 1.0f, 1.0f} });
			cmdBuffer->BindPipeline(i, pipeline);
			cmdBuffer->BindDescriptorSets(i, { descriptorSet }, pipeline);
			cmdBuffer->BindVertexBuffers(i, { vbv });
			cmdBuffer->BindIndexBuffer(i, ibv);
			cmdBuffer->DrawIndexed(i, 6);
			cmdBuffer->EndRenderPass(i);
			cmdBuffer->End(i);
		}
	};
	RecordPresentCmdBuffers();

	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "DrawFence";
	fenceCI.device = context->GetDevice();
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<Ref<Fence>>draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
	Semaphore::CreateInfo semaphoreCI = { "Seamphore", context->GetDevice() };
	std::vector<Ref<Semaphore>>acquire = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };
	std::vector<Ref<Semaphore>>submit = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };

	uint32_t frameIndex = 0;
	MSG msg = { 0 };
	//Main Render Loop
	while (!g_WindowQuit)
	{
		WindowUpdate();

		if (shaderRecompile)
		{
			context->DeviceWaitIdle();
			vertexShader->Recompile();
			fragmentShader->Recompile();

			pCI.shaders = { vertexShader, fragmentShader };
			pipeline = Pipeline::Create(&pCI);

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
			RecordPresentCmdBuffers();

			shaderRecompile = false;
		}
		if (swapchain->m_Resized)
		{
			swapchain->Resize(width, height);
			
			pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
			pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
			pipeline = Pipeline::Create(&pCI);

			framebufferCI_0.attachments = { swapchain->m_SwapchainImageViews[0] };
			framebufferCI_0.width = width;
			framebufferCI_0.height = height;
			framebuffer0 = Framebuffer::Create(&framebufferCI_0);

			framebufferCI_1.attachments = { swapchain->m_SwapchainImageViews[1] };
			framebufferCI_1.width = width;
			framebufferCI_1.height = height;
			framebuffer1 = Framebuffer::Create(&framebufferCI_1);

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
			RecordPresentCmdBuffers();

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquire = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };
			submit = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };

			swapchain->m_Resized = false;
		}
		cmdBuffer->Present({ 0, 1 }, swapchain, draws, acquire, submit, windowResize);
	}
	context->DeviceWaitIdle();
}