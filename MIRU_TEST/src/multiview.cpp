#include "miru_core.h"
#include "common.h"
#include "maths.h"

#include "stb/stb_image.h"

using namespace miru;
using namespace base;

static bool g_WindowQuit = false;
static uint32_t width = 800*2;
static uint32_t height = 600;
static bool windowResize = false;
static bool shaderRecompile = false;
static int var_x, var_y, var_z = 0;

static HWND window;
static LRESULT CALLBACK WindProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
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
		windowResize = true;
	}
	if (msg == WM_KEYDOWN && wparam == 0x52) //R
	{
		shaderRecompile = true;
	}
	if (msg == WM_KEYDOWN && wparam == 0x49) //I
		var_y--;
	if (msg == WM_KEYDOWN && wparam == 0x4B) //K
		var_y++;
	if (msg == WM_KEYDOWN && wparam == 0x4A) //J
		var_x--;
	if (msg == WM_KEYDOWN && wparam == 0x4C) //L
		var_x++;
	if (msg == WM_KEYDOWN && wparam == 0x48) //H
		var_z--;
	if (msg == WM_KEYDOWN && wparam == 0x4E) //N
		var_z++;

	return DefWindowProc(handle, msg, wparam, lparam);
}
static void WindowUpdate()
{
	MSG msg = { 0 };
	if (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Multiview()
{
	GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
	//GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
	GraphicsAPI::AllowSetName();
	GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::PIX);

	MIRU_CPU_PROFILE_BEGIN_SESSION("miru_profile_result.txt");

	Context::CreateInfo contextCI;
	contextCI.applicationName = "MIRU_TEST";
	contextCI.extensions = Context::ExtensionsBit::MULTIVIEW;
	contextCI.debugValidationLayers = true;
	contextCI.deviceDebugName = "GPU Device";
	contextCI.pNext = nullptr;
	ContextRef context = Context::Create(&contextCI);
	
	//Creates the windows
	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = contextCI.applicationName.c_str();
	RegisterClass(&wc);

	window = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, 100, 100, width, height, 0, 0, 0, 0);
	ShowWindow(window, SW_SHOW);

	Swapchain::CreateInfo swapchainCI;
	swapchainCI.debugName = "Swapchain";
	swapchainCI.context = context;
	swapchainCI.pWindow = window;
	swapchainCI.width = width;
	swapchainCI.height = height;
	swapchainCI.swapchainCount = 2;
	swapchainCI.vSync = true;
	SwapchainRef swapchain = Swapchain::Create(&swapchainCI);
	width = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().image->GetCreateInfo().width;
	height = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().image->GetCreateInfo().height;

	//Multiview shader
	auto compileArguments = base::Shader::LoadCompileArgumentsFromFile("../shaderbin/multiview_hlsl.json", { { "$SOLUTION_DIR", SOLUTION_DIR }, { "$BUILD_DIR", BUILD_DIR } });
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "Multiview: Vertex Shader Module";
	shaderCI.device = context->GetDevice();
	shaderCI.stageAndEntryPoints = { {Shader::StageBit::VERTEX_BIT, "vs_main"} };
	shaderCI.binaryFilepath = "../shaderbin/multiview_vs_6_1_vs_main.spv";
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments = compileArguments[0];
	ShaderRef vertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Multiview: Fragment Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::PIXEL_BIT, "ps_main"} };
	shaderCI.binaryFilepath = "../shaderbin/multiview_ps_6_1_ps_main.spv";
	shaderCI.recompileArguments = compileArguments[1];
	ShaderRef fragmentShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Multiview Show: Vertex Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::VERTEX_BIT, "vs_show"} };
	shaderCI.binaryFilepath = "../shaderbin/multiview_vs_6_1_vs_show.spv";
	shaderCI.recompileArguments = compileArguments[2];
	ShaderRef showVertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Multiview Show: Fragment Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::PIXEL_BIT, "ps_show"} };
	shaderCI.binaryFilepath = "../shaderbin/multiview_ps_6_1_ps_show.spv";
	shaderCI.recompileArguments = compileArguments[3];
	ShaderRef showFragmentShader = Shader::Create(&shaderCI);

	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CmdPool";
	cmdPoolCI.context = context;
	cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
	CommandPoolRef cmdPool = CommandPool::Create(&cmdPoolCI);
	cmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
	CommandPoolRef cmdCopyPool = CommandPool::Create(&cmdPoolCI);

	CommandBuffer::CreateInfo cmdBufferCI, cmdCopyBufferCI;
	cmdBufferCI.debugName = "CmdBuffer";
	cmdBufferCI.commandPool = cmdPool;
	cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdBufferCI.commandBufferCount = 3;
	CommandBufferRef cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
	cmdCopyBufferCI.debugName = "CmdCopyBuffer";
	cmdCopyBufferCI.commandPool = cmdCopyPool;
	cmdCopyBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdCopyBufferCI.commandBufferCount = 1;
	CommandBufferRef cmdCopyBuffer = CommandBuffer::Create(&cmdCopyBufferCI);

	Allocator::CreateInfo allocCI;
	allocCI.debugName = "CPU_ALLOC_0";
	allocCI.context = context;
	allocCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
	allocCI.properties = Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT;
	AllocatorRef cpu_alloc_0 = Allocator::Create(&allocCI);
	allocCI.debugName = "GPU_ALLOC_0";
	allocCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	AllocatorRef gpu_alloc_0 = Allocator::Create(&allocCI);

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
	std::string logoFilepath = std::string(SOLUTION_DIR) + std::string("/Branding/logo.png");
	uint8_t* imageData = stbi_load(logoFilepath.c_str(), &img_width, &img_height, &bpp, 4);

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices Buffer";
	verticesBufferCI.device = context->GetDevice();
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	verticesBufferCI.size = sizeof(vertices);
	verticesBufferCI.data = vertices;
	verticesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_vb = Buffer::Create(&verticesBufferCI);
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT;
	verticesBufferCI.data = nullptr;
	verticesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_vb = Buffer::Create(&verticesBufferCI);

	Buffer::CreateInfo indicesBufferCI;
	indicesBufferCI.debugName = "Indices Buffer";
	indicesBufferCI.device = context->GetDevice();
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_ib = Buffer::Create(&indicesBufferCI);
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	indicesBufferCI.data = nullptr;
	indicesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_ib = Buffer::Create(&indicesBufferCI);

	Buffer::CreateInfo imageBufferCI;
	imageBufferCI.debugName = "MIRU logo upload buffer";
	imageBufferCI.device = context->GetDevice();
	imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	imageBufferCI.size = img_width * img_height * 4;
	imageBufferCI.data = imageData;
	imageBufferCI.allocator = cpu_alloc_0;
	BufferRef c_imageBuffer = Buffer::Create(&imageBufferCI);
	stbi_image_free(imageData);

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
	imageCI.allocator = gpu_alloc_0;
	imageCI.externalImage = nullptr;
	ImageRef image = Image::Create(&imageCI);

	Mat4 proj = Mat4::Perspective(3.14159 / 2.0, float(width) / float(height), 0.1f, 100.0f);
	Mat4 view = Mat4::Identity();
	Mat4 modl0 = Mat4::Translation({ -0.5f, 0.0f, -1.5f });
	Mat4 modl1 = Mat4::Translation({ +0.5f, 0.0f, -1.5f });

	float ubData[2 * sizeof(Mat4)];
	memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
	memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

	float ubData2[2 * sizeof(Mat4)];
	memcpy(ubData2 + 0 * 16, &modl0.a, sizeof(Mat4));
	memcpy(ubData2 + 1 * 16, &modl1.a, sizeof(Mat4));

	Buffer::CreateInfo ubCI;
	ubCI.debugName = "Camera UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = 2 * sizeof(Mat4);
	ubCI.data = ubData;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ub1 = Buffer::Create(&ubCI);
	ubCI.debugName = "Model UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = 2 * sizeof(Mat4);
	ubCI.data = ubData2;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ub2 = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubViewCamCI;
	ubViewCamCI.debugName = "Camera UBView";
	ubViewCamCI.device = context->GetDevice();
	ubViewCamCI.type = BufferView::Type::UNIFORM;
	ubViewCamCI.buffer = ub1;
	ubViewCamCI.offset = 0;
	ubViewCamCI.size = 2 * sizeof(Mat4);
	ubViewCamCI.stride = 0;
	BufferViewRef ubViewCam = BufferView::Create(&ubViewCamCI);

	BufferView::CreateInfo ubViewMdlCI;
	ubViewMdlCI.debugName = "Model UBView";
	ubViewMdlCI.device = context->GetDevice();
	ubViewMdlCI.type = BufferView::Type::UNIFORM;
	ubViewMdlCI.buffer = ub2;
	ubViewMdlCI.offset = 0;
	ubViewMdlCI.size = 2 * sizeof(Mat4);
	ubViewMdlCI.stride = 0;
	BufferViewRef ubViewMdl = BufferView::Create(&ubViewMdlCI);

	//Transfer CmdBuffer
	Fence::CreateInfo transferFenceCI = { "TransferFence", context->GetDevice(), false, UINT64_MAX };
	FenceRef transferFence = Fence::Create(&transferFenceCI);
	Semaphore::CreateInfo transferSemaphoreCI = { "TransferSemaphore", context->GetDevice() };
	SemaphoreRef transferSemaphore = Semaphore::Create(&transferSemaphoreCI);
	{
		cmdCopyBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		cmdCopyBuffer->CopyBuffer(0, c_vb, g_vb, { { 0, 0, sizeof(vertices) } });
		cmdCopyBuffer->CopyBuffer(0, c_ib, g_ib, { { 0, 0, sizeof(indices) } });

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
		bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.srcQueueFamilyIndex =Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex =Barrier::QueueFamilyIgnored;
		bCI.image = image;
		bCI.oldLayout = Image::Layout::UNKNOWN;
		bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
		BarrierRef b = Barrier::Create(&bCI);
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
	CommandBuffer::SubmitInfo copySI = { { 0 }, {}, {}, {}, { transferSemaphore }, {} };
	cmdCopyBuffer->Submit({ copySI }, nullptr);
	{
		cmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.image = image;
		bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
		BarrierRef b = Barrier::Create(&bCI);
		cmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, { b });

		cmdBuffer->End(2);
	}
	CommandBuffer::SubmitInfo copy2SI = { { 2 }, { transferSemaphore }, {}, { PipelineStageBit::TRANSFER_BIT }, {}, {} };
	cmdBuffer->Submit({ copy2SI }, transferFence);
	transferFence->Wait();

	BufferView::CreateInfo vbViewCI;
	vbViewCI.debugName = "VerticesBufferView";
	vbViewCI.device = context->GetDevice();
	vbViewCI.type = BufferView::Type::VERTEX;
	vbViewCI.buffer = g_vb;
	vbViewCI.offset = 0;
	vbViewCI.size = sizeof(vertices);
	vbViewCI.stride = 4 * sizeof(float);
	BufferViewRef vbv = BufferView::Create(&vbViewCI);

	BufferView::CreateInfo ibViewCI;
	ibViewCI.debugName = "IndicesBufferView";
	ibViewCI.device = context->GetDevice();
	ibViewCI.type = BufferView::Type::INDEX;
	ibViewCI.buffer = g_ib;
	ibViewCI.offset = 0;
	ibViewCI.size = sizeof(indices);
	ibViewCI.stride = sizeof(uint32_t);
	BufferViewRef ibv = BufferView::Create(&ibViewCI);

	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "MIRU logo ImageView";
	imageViewCI.device = context->GetDevice();
	imageViewCI.image = image;
	imageViewCI.viewType = Image::Type::TYPE_CUBE;
	imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
	ImageViewRef imageView = ImageView::Create(&imageViewCI);

	Sampler::CreateInfo samplerCI;
	samplerCI.debugName = "Default Sampler";
	samplerCI.device = context->GetDevice();
	samplerCI.magFilter = Sampler::Filter::LINEAR;
	samplerCI.minFilter = Sampler::Filter::LINEAR;
	samplerCI.mipmapMode = Sampler::MipmapMode::LINEAR;
	samplerCI.addressModeU = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.addressModeV = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.addressModeW = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.mipLodBias = 1;
	samplerCI.anisotropyEnable = false;
	samplerCI.maxAnisotropy = 1.0f;
	samplerCI.compareEnable = false;
	samplerCI.compareOp = CompareOp::NEVER;
	samplerCI.minLod = 0;
	samplerCI.maxLod = 1;
	samplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	samplerCI.unnormalisedCoordinates = false;
	SamplerRef sampler = Sampler::Create(&samplerCI);

	//Colour
	Image::CreateInfo colourCI;
	colourCI.debugName = "Colour Image";
	colourCI.device = context->GetDevice();
	colourCI.type = Image::Type::TYPE_2D_ARRAY;
	colourCI.format = swapchain->m_SwapchainImages[0]->GetCreateInfo().format;
	colourCI.width = width / 2;
	colourCI.height = height;
	colourCI.depth = 1;
	colourCI.mipLevels = 1;
	colourCI.arrayLayers = 2;
	colourCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	colourCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::SAMPLED_BIT;
	colourCI.layout = Image::Layout::UNKNOWN;
	colourCI.size = 0;
	colourCI.data = nullptr;
	colourCI.allocator = gpu_alloc_0;
	colourCI.externalImage = nullptr;
	ImageRef colourImage = Image::Create(&colourCI);

	ImageView::CreateInfo colourImageViewCI;
	colourImageViewCI.debugName = "Colour ImageView";
	colourImageViewCI.device = context->GetDevice();
	colourImageViewCI.image = colourImage;
	colourImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
	colourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 2 };
	ImageViewRef colourImageView = ImageView::Create(&colourImageViewCI);

	//Depth
	Image::CreateInfo depthCI;
	depthCI.debugName = "Depth Image";
	depthCI.device = context->GetDevice();
	depthCI.type = Image::Type::TYPE_2D_ARRAY;
	depthCI.format = Image::Format::D32_SFLOAT;
	depthCI.width = width / 2;
	depthCI.height = height;
	depthCI.depth = 1;
	depthCI.mipLevels = 1;
	depthCI.arrayLayers = 2;
	depthCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	depthCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	depthCI.layout = Image::Layout::UNKNOWN;
	depthCI.size = 0;
	depthCI.data = nullptr;
	depthCI.allocator = gpu_alloc_0;
	depthCI.externalImage = nullptr;
	ImageRef depthImage = Image::Create(&depthCI);

	ImageView::CreateInfo depthImageViewCI;
	depthImageViewCI.debugName = "Depth ImageView";
	depthImageViewCI.device = context->GetDevice();
	depthImageViewCI.image = depthImage;
	depthImageViewCI.viewType = Image::Type::TYPE_2D_ARRAY;
	depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 2 };
	ImageViewRef depthImageView = ImageView::Create(&depthImageViewCI);

	//Basic Descriptor sets
	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "Basic: Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 2},  {DescriptorType::UNIFORM_BUFFER, 2} };
	descriptorPoolCI.maxSets = 3;
	DescriptorPoolRef descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "Basic: DescSetLayout1";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT } };
	DescriptorSetLayoutRef setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "Basic: DescSetLayout2";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT },
		{1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT }
	};
	DescriptorSetLayoutRef setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "Show: DescSetLayout0";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT }
	};
	DescriptorSetLayoutRef setLayout3 = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetCI;
	descriptorSetCI.debugName = "Basic: Descriptor Set 0";
	descriptorSetCI.descriptorPool = descriptorPool;
	descriptorSetCI.descriptorSetLayouts = { setLayout1 };
	DescriptorSetRef descriptorSet_p0 = DescriptorSet::Create(&descriptorSetCI);
	descriptorSetCI.debugName = "Basic: Descriptor Set 1";
	descriptorSetCI.descriptorSetLayouts = { setLayout2 };
	DescriptorSetRef descriptorSet_p1 = DescriptorSet::Create(&descriptorSetCI);
	descriptorSetCI.debugName = "Show: Descriptor Set 0";
	descriptorSetCI.descriptorSetLayouts = { setLayout3 };
	DescriptorSetRef descriptorSet_p2 = DescriptorSet::Create(&descriptorSetCI);
	descriptorSet_p0->AddBuffer(0, 0, { { ubViewCam } });
	descriptorSet_p1->AddBuffer(0, 0, { { ubViewMdl } });
	descriptorSet_p1->AddImage(0, 1, { { sampler, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descriptorSet_p2->AddImage(0, 0, { { sampler, colourImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descriptorSet_p0->Update();
	descriptorSet_p1->Update();
	descriptorSet_p2->Update();
	
	//Main RenderPass
	RenderPass::CreateInfo renderPassCI;
	renderPassCI.debugName = "Basic: RenderPass";
	renderPassCI.device = context->GetDevice();
	renderPassCI.attachments = {
		{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,						//Colour
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::STORE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::SHADER_READ_ONLY_OPTIMAL
		},
		{depthImage->GetCreateInfo().format,											//Depth
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};
	renderPassCI.subpassDescriptions = {
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {} },
	};
	renderPassCI.subpassDependencies = {
		{RenderPass::SubpassExternal, 0,
		PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
	};
	renderPassCI.multiview = { { 0b11 }, {}, { 0b11 } };
	RenderPassRef renderPass = RenderPass::Create(&renderPassCI);

	//View RenderPass
	renderPassCI.debugName = "Show: RenderPass";
	renderPassCI.attachments = {
		{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,						//Swapchain
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::STORE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::PRESENT_SRC
		}
	};
	renderPassCI.subpassDescriptions = {
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {}, {} },
	};
	renderPassCI.subpassDependencies = {
		{RenderPass::SubpassExternal, 0,
		PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
	};
	renderPassCI.multiview = {};
	RenderPassRef showRenderPass = RenderPass::Create(&renderPassCI);

	//Basic pipeline
	Pipeline::CreateInfo pCI;
	pCI.debugName = "Basic";
	pCI.device = context->GetDevice();
	pCI.type = PipelineType::GRAPHICS;
	pCI.shaders = { vertexShader, fragmentShader };
	pCI.vertexInputState.vertexInputBindingDescriptions = { {0, sizeof(vertices) / 8, VertexInputRate::VERTEX} };
	pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0, "POSITION"} };
	pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
	pCI.tessellationState = {};
	pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width / 2, (float)height, 0.0f, 1.0f} };
	pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width / 2, height}} };
	pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, UINT32_MAX, false, false };
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
	pCI.layout = { { setLayout1, setLayout2 }, {} };
	pCI.renderPass = renderPass;
	pCI.subpassIndex = 0;
	PipelineRef pipeline = Pipeline::Create(&pCI);

	//Show pipeline
	Pipeline::CreateInfo pShowCI;
	pShowCI.debugName = "Show";
	pShowCI.device = context->GetDevice();
	pShowCI.type = PipelineType::GRAPHICS;
	pShowCI.shaders = { showVertexShader, showFragmentShader };
	pShowCI.vertexInputState.vertexInputBindingDescriptions = {};
	pShowCI.vertexInputState.vertexInputAttributeDescriptions = {};
	pShowCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
	pShowCI.tessellationState = {};
	pShowCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
	pShowCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
	pShowCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pShowCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, UINT32_MAX, false, false };
	pShowCI.depthStencilState = { false, false, CompareOp::NEVER, false, false, {}, {}, 0.0f, 1.0f };
	pShowCI.colourBlendState.logicOpEnable = false;
	pShowCI.colourBlendState.logicOp = LogicOp::COPY;
	pShowCI.colourBlendState.attachments = { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
											BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } };
	pShowCI.colourBlendState.blendConstants[0] = 0.0f;
	pShowCI.colourBlendState.blendConstants[1] = 0.0f;
	pShowCI.colourBlendState.blendConstants[2] = 0.0f;
	pShowCI.colourBlendState.blendConstants[3] = 0.0f;
	pShowCI.dynamicStates = {};
	pShowCI.layout = { { setLayout3, }, {} };
	pShowCI.renderPass = showRenderPass;
	pShowCI.subpassIndex = 0;
	PipelineRef showPipeline = Pipeline::Create(&pShowCI);

	Framebuffer::CreateInfo framebufferCI;
	framebufferCI.debugName = "Framebuffer0";
	framebufferCI.device = context->GetDevice();
	framebufferCI.renderPass = renderPass;
	framebufferCI.attachments = { colourImageView, depthImageView };
	framebufferCI.width = width / 2;
	framebufferCI.height = height;
	framebufferCI.layers = 1;
	FramebufferRef framebuffer = Framebuffer::Create(&framebufferCI);

	Framebuffer::CreateInfo showFramebufferCI_0, showFramebufferCI_1;
	showFramebufferCI_0.debugName = "Show Framebuffer0";
	showFramebufferCI_0.device = context->GetDevice();
	showFramebufferCI_0.renderPass = showRenderPass;
	showFramebufferCI_0.attachments = { swapchain->m_SwapchainImageViews[0] };
	showFramebufferCI_0.width = width;
	showFramebufferCI_0.height = height;
	showFramebufferCI_0.layers = 1;
	FramebufferRef showFramebuffer0 = Framebuffer::Create(&showFramebufferCI_0);
	showFramebufferCI_1.debugName = "Show Framebuffer1";
	showFramebufferCI_1.device = context->GetDevice();
	showFramebufferCI_1.renderPass = showRenderPass;
	showFramebufferCI_1.attachments = { swapchain->m_SwapchainImageViews[1] };
	showFramebufferCI_1.width = width;
	showFramebufferCI_1.height = height;
	showFramebufferCI_1.layers = 1;
	FramebufferRef showFramebuffer1 = Framebuffer::Create(&showFramebufferCI_1);

	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "DrawFence";
	fenceCI.device = context->GetDevice();
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<FenceRef> draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
	Semaphore::CreateInfo acquireSemaphoreCI = { "AcquireSeamphore", context->GetDevice() };
	Semaphore::CreateInfo submitSemaphoreCI = { "SubmitSeamphore", context->GetDevice() };
	std::vector<SemaphoreRef> acquires = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
	std::vector<SemaphoreRef> submits = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };


	MIRU_CPU_PROFILE_END_SESSION();

	uint32_t frameIndex = 0;
	uint32_t frameCount = 0;
	uint32_t swapchainImageIndex = 0;
	float r = 1.00f;
	float g = 0.00f;
	float b = 0.00f;
	float increment = 1.0f / 60.0f;
	
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

			showVertexShader->Recompile();
			showFragmentShader->Recompile();
			pShowCI.shaders = { showVertexShader, showFragmentShader };
			showPipeline = Pipeline::Create(&pShowCI);

			shaderRecompile = false;
		}
		if (swapchain->m_Resized || windowResize)
		{
			swapchain->Resize(width, height);

			pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width / 2, (float)height, 0.0f, 1.0f} };
			pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width / 2, height}} };
			pipeline = Pipeline::Create(&pCI);

			pShowCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
			pShowCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
			showPipeline = Pipeline::Create(&pShowCI);

			colourCI.width = width / 2;
			colourCI.height = height;
			colourImage = Image::Create(&colourCI);
			colourImageViewCI.image = colourImage;
			colourImageView = ImageView::Create(&colourImageViewCI);

			depthCI.width = width / 2;
			depthCI.height = height;
			depthImage = Image::Create(&depthCI);
			depthImageViewCI.image = depthImage;
			depthImageView = ImageView::Create(&depthImageViewCI);

			descriptorSet_p2 = nullptr;
			descriptorSetCI.descriptorSetLayouts = { setLayout3 };
			descriptorSet_p2 = DescriptorSet::Create(&descriptorSetCI);
			descriptorSet_p2->AddImage(0, 0, { { sampler, colourImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			descriptorSet_p2->Update();

			framebufferCI.attachments = { colourImageView, depthImageView };
			framebufferCI.width = width / 2;
			framebufferCI.height = height;
			framebuffer = Framebuffer::Create(&framebufferCI);

			showFramebufferCI_0.attachments = {swapchain->m_SwapchainImageViews[0] };
			showFramebufferCI_0.width = width;
			showFramebufferCI_0.height = height;
			showFramebuffer0 = Framebuffer::Create(&showFramebufferCI_0);

			showFramebufferCI_1.attachments = { swapchain->m_SwapchainImageViews[1] };
			showFramebufferCI_1.width = width;
			showFramebufferCI_1.height = height;
			showFramebuffer1 = Framebuffer::Create(&showFramebufferCI_1);

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquires = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
			submits = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };

			swapchain->m_Resized = false;
			windowResize = false;
		}

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
			draws[frameIndex]->Reset();

			swapchain->AcquireNextImage(acquires[frameIndex], swapchainImageIndex);

			cmdBuffer->Reset(frameIndex, false);
			cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);

			cmdBuffer->BeginDebugLabel(frameIndex, "Multiview");
			cmdBuffer->BeginRenderPass(frameIndex, framebuffer, { {r, g, b, 1.0f}, {0.0f, 0} });
			cmdBuffer->BindPipeline(frameIndex, pipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet_p0 }, 0, pipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet_p1 }, 1, pipeline);
			cmdBuffer->BindVertexBuffers(frameIndex, { vbv });
			cmdBuffer->BindIndexBuffer(frameIndex, ibv);
			cmdBuffer->DrawIndexed(frameIndex, 36);
			cmdBuffer->EndRenderPass(frameIndex);
			cmdBuffer->EndDebugLabel(frameIndex);

			cmdBuffer->BeginDebugLabel(frameIndex, "Show");
			cmdBuffer->BeginRenderPass(frameIndex, swapchainImageIndex == 0 ? showFramebuffer0 : showFramebuffer1, { {0.0f, 0.0f, 0.0f, 1.0f} });
			cmdBuffer->BindPipeline(frameIndex, showPipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet_p2 }, 0, showPipeline);
			cmdBuffer->Draw(frameIndex, 12);
			cmdBuffer->EndRenderPass(frameIndex);
			cmdBuffer->EndDebugLabel(frameIndex);

			cmdBuffer->End(frameIndex);

			CommandBuffer::SubmitInfo mainSI = { { frameIndex }, { acquires[frameIndex] }, {}, { base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT }, { submits[frameIndex] }, {} };
			cmdBuffer->Submit({ mainSI }, draws[frameIndex]);

			swapchain->Present(cmdPool, submits[frameIndex], swapchainImageIndex);

			proj = Mat4::Perspective(3.14159 / 2.0, float(width) / float(height), 0.1f, 100.0f);
			if (GraphicsAPI::IsVulkan())
				proj.f *= -1;
			/*modl = Mat4::Translation({ 0.0f, 0.0f, -1.5f })
				//* translate(mat4(1.0f), { float(var_x)/10.0f, float(var_y)/10.0f, float(var_z)/10.0f})
				* Mat4::Rotation((var_x * 5.0f) * 3.14159 / 180.0, { 0, 1, 0 })
				* Mat4::Rotation((var_y * 5.0f) * 3.14159 / 180.0, { 1, 0, 0 })
				* Mat4::Rotation((var_z * 5.0f) * 3.14159 / 180.0, { 0, 0, 1 });*/

			memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
			memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

			cpu_alloc_0->SubmitData(ub1->GetAllocation(), 0, 2 * sizeof(Mat4), ubData);
			//cpu_alloc_0->SubmitData(ub2->GetAllocation(), 0, sizeof(Mat4), (void*)&modl.a);

			frameIndex = (frameIndex + 1) % 2;
			frameCount++;
		}
	}
	context->DeviceWaitIdle();
}