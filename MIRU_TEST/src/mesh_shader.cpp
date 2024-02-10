#include "miru_core.h"
#include "common.h"
#include "maths.h"

#include "stb/stb_image.h"

using namespace miru;
using namespace base;

static bool g_WindowQuit = false;
static uint32_t width = 800;
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

void MeshShader()
{
	GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
	//GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
	GraphicsAPI::AllowSetName();
	GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::RENDER_DOC);

	MIRU_CPU_PROFILE_BEGIN_SESSION("miru_profile_result.txt");

	Context::CreateInfo contextCI;
	contextCI.applicationName = "MIRU_TEST";
	contextCI.extensions = Context::ExtensionsBit::MESH_SHADER;
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

	//Basic shader
	auto compileArguments = base::Shader::LoadCompileArgumentsFromFile("../shaderbin/meshshader_hlsl.json", { { "$SOLUTION_DIR", SOLUTION_DIR }, { "$BUILD_DIR", BUILD_DIR } });
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "MeshShader: Mesh Shader Module";
	shaderCI.device = context->GetDevice();
	shaderCI.stageAndEntryPoints = { {Shader::StageBit::MESH_BIT_EXT, "ms_main"} };
	shaderCI.binaryFilepath = "../shaderbin/meshshader_ms_6_5_ms_main.spv";
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments = compileArguments[0];
	ShaderRef meshShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "MeshShader: Fragment Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::PIXEL_BIT, "ps_main"} };
	shaderCI.binaryFilepath = "../shaderbin/meshshader_ps_6_2_ps_main.spv";
	shaderCI.recompileArguments = compileArguments[1];
	ShaderRef fragmentShader = Shader::Create(&shaderCI);

	//PostProcessing
	compileArguments.clear();
	compileArguments = base::Shader::LoadCompileArgumentsFromFile("../shaderbin/postprocess_hlsl.json", { { "$SOLUTION_DIR", SOLUTION_DIR }, { "$BUILD_DIR", BUILD_DIR } });
	shaderCI.debugName = "PostProcess: Vertex Shader Module";
	shaderCI.stageAndEntryPoints = { {Shader::StageBit::VERTEX_BIT, "vs_main"} };
	shaderCI.binaryFilepath = "../shaderbin/postprocess_vs_6_0_vs_main.spv";
	shaderCI.recompileArguments = compileArguments[0];
	ShaderRef postProcessVertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "PostProcess: Fragment Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::PIXEL_BIT, "ps_main"} };
	shaderCI.binaryFilepath = "../shaderbin/postprocess_ps_6_0_ps_main.spv";
	shaderCI.recompileArguments = compileArguments[1];
	ShaderRef postProcessFragmentShader = Shader::Create(&shaderCI);

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

	Vec4 vertices[8] =
	{
		{ -0.5f, -0.5f, -0.5f, 1.0f },
		{ +0.5f, -0.5f, -0.5f, 1.0f },
		{ +0.5f, +0.5f, -0.5f, 1.0f },
		{ -0.5f, +0.5f, -0.5f, 1.0f },
		{ -0.5f, -0.5f, +0.5f, 1.0f },
		{ +0.5f, -0.5f, +0.5f, 1.0f },
		{ +0.5f, +0.5f, +0.5f, 1.0f },
		{ -0.5f, +0.5f, +0.5f, 1.0f }
	};
	uint32_t indices[36] = {
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		5, 4, 7, 7, 6, 5,
		4, 0, 3, 3, 7, 4,
		3, 2, 6, 6, 7, 3,
		4, 5, 1, 1, 0, 4
	};
	struct Meshlet
	{
		uint32_t vertices[64]; //Index into the vertex buffer.
		uint32_t indices[126 * 3]; //Up to 126 triangles.
		uint32_t indexCount;
		uint32_t vertexCount;
	};
	std::vector<Meshlet> meshlets;

	//https://github.com/zeux/niagara/commit/7ad941833f5bd23f19ea667a3b2cc3911520d20b
	{
		Meshlet meshlet = {};
		std::vector<uint16_t> meshletVertices(_countof(vertices), 0xFF);

		for (size_t i = 0; i < _countof(indices); i += 3)
		{
			//Get triangle indices
			uint32_t index_a = indices[i + 0];
			uint32_t index_b = indices[i + 1];
			uint32_t index_c = indices[i + 2];

			//Get triangle vertices
			uint16_t& vertex_a = meshletVertices[index_a];
			uint16_t& vertex_b = meshletVertices[index_b];
			uint16_t& vertex_c = meshletVertices[index_c];

			//Check if we can fit this triangle into the current mesh, if not get a new meshlet
			bool outOfVertices = (meshlet.vertexCount + uint32_t(vertex_a == 0xFF) + uint32_t(vertex_b == 0xFF) + uint32_t(vertex_c == 0xFF)) > _countof(Meshlet::vertices);
			bool outOfIndices = (meshlet.indexCount + 3) > _countof(Meshlet::indices);
			if (outOfVertices || outOfIndices)
			{
				meshlets.push_back(meshlet);
				meshletVertices.assign(meshletVertices.size(), 0xFF);
				meshlet = {};
			}

			//Add vertices to the meshlet
			if (vertex_a == 0xFF)
			{
				vertex_a = meshlet.vertexCount;
				meshlet.vertices[meshlet.vertexCount++] = index_a;
			}
			if (vertex_b == 0xFF)
			{
				vertex_b = meshlet.vertexCount;
				meshlet.vertices[meshlet.vertexCount++] = index_b;
			}
			if (vertex_c == 0xFF)
			{
				vertex_c = meshlet.vertexCount;
				meshlet.vertices[meshlet.vertexCount++] = index_c;
			}

			//Add indices to meshlet
			meshlet.indices[meshlet.indexCount++] = vertex_a;
			meshlet.indices[meshlet.indexCount++] = vertex_b;
			meshlet.indices[meshlet.indexCount++] = vertex_c;
		}

		//Push final meshlet
		if (meshlet.indexCount)
			meshlets.push_back(meshlet);
	}

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
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::STORAGE_BIT;
	verticesBufferCI.data = nullptr;
	verticesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_vb = Buffer::Create(&verticesBufferCI);

	Buffer::CreateInfo meshletsBufferCI;
	meshletsBufferCI.debugName = "Meshlets Buffer";
	meshletsBufferCI.device = context->GetDevice();
	meshletsBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	meshletsBufferCI.size = sizeof(Meshlet) * meshlets.size();
	meshletsBufferCI.data = meshlets.data();
	meshletsBufferCI.allocator = cpu_alloc_0;
	BufferRef c_mb = Buffer::Create(&meshletsBufferCI);
	meshletsBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::STORAGE_BIT;
	meshletsBufferCI.data = nullptr;
	meshletsBufferCI.allocator = gpu_alloc_0;
	BufferRef g_mb = Buffer::Create(&meshletsBufferCI);

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
	Mat4 modl = Mat4::Translation({ 0.0f, 0.0f, -1.5f });

	float ubData[2 * sizeof(Mat4)];
	memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
	memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

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
	ubCI.size = sizeof(Mat4);
	ubCI.data = &modl.a;
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
	ubViewMdlCI.size = sizeof(Mat4);
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
		cmdCopyBuffer->CopyBuffer(0, c_mb, g_mb, { { 0, 0, sizeof(Meshlet) * meshlets.size() } });

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
		bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
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
	vbViewCI.type = BufferView::Type::STORAGE;
	vbViewCI.buffer = g_vb;
	vbViewCI.offset = 0;
	vbViewCI.size = sizeof(vertices);
	vbViewCI.stride = sizeof(Vec4);
	BufferViewRef vbv = BufferView::Create(&vbViewCI);

	BufferView::CreateInfo mbViewCI;
	mbViewCI.debugName = "MeshletsBufferView";
	mbViewCI.device = context->GetDevice();
	mbViewCI.type = BufferView::Type::STORAGE;
	mbViewCI.buffer = g_mb;
	mbViewCI.offset = 0;
	mbViewCI.size = sizeof(Meshlet) * meshlets.size();
	mbViewCI.stride = sizeof(Meshlet);
	BufferViewRef mbv = BufferView::Create(&mbViewCI);

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
	SamplerRef sampler = Sampler::Create(&samplerCI);

	//Colour MSAA
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
	colourImageViewCI.viewType = Image::Type::TYPE_2D;
	colourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	ImageViewRef colourImageView = ImageView::Create(&colourImageViewCI);

	//Depth MSAA
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
	depthImageViewCI.viewType = Image::Type::TYPE_2D;
	depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
	ImageViewRef depthImageView = ImageView::Create(&depthImageViewCI);

	//Resolve and Input
	Image::CreateInfo resolveAndInputImageCI;
	resolveAndInputImageCI.debugName = "Resolve and Input";
	resolveAndInputImageCI.device = context->GetDevice();
	resolveAndInputImageCI.type = Image::Type::TYPE_2D;;
	resolveAndInputImageCI.format = swapchain->m_SwapchainImages[0]->GetCreateInfo().format;
	resolveAndInputImageCI.width = width;
	resolveAndInputImageCI.height = height;
	resolveAndInputImageCI.depth = 1;
	resolveAndInputImageCI.mipLevels = 1;
	resolveAndInputImageCI.arrayLayers = 1;
	resolveAndInputImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	resolveAndInputImageCI.usage = Image::UsageBit::COLOUR_ATTACHMENT_BIT | Image::UsageBit::INPUT_ATTACHMENT_BIT;
	resolveAndInputImageCI.layout = Image::Layout::UNKNOWN;
	resolveAndInputImageCI.size = 0;
	resolveAndInputImageCI.data = nullptr;
	resolveAndInputImageCI.allocator = gpu_alloc_0;
	resolveAndInputImageCI.externalImage = nullptr;
	ImageRef resolveAndInputImage = Image::Create(&resolveAndInputImageCI);

	ImageView::CreateInfo resolveAndInputImageViewCI;
	resolveAndInputImageViewCI.debugName = "Resolve and Input ImageView";
	resolveAndInputImageViewCI.device = context->GetDevice();
	resolveAndInputImageViewCI.image = resolveAndInputImage;
	resolveAndInputImageViewCI.viewType = Image::Type::TYPE_2D;
	resolveAndInputImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	ImageViewRef resolveAndInputImageView = ImageView::Create(&resolveAndInputImageViewCI);

	//Basic and Pipeline Descriptor sets
	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "Basic: Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1}, {DescriptorType::UNIFORM_BUFFER, 2}, {DescriptorType::STORAGE_BUFFER, 2}, {DescriptorType::INPUT_ATTACHMENT, 1} };
	descriptorPoolCI.maxSets = 3;
	DescriptorPoolRef descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "Basic: DescSetLayout1";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::MESH_BIT_EXT } };
	DescriptorSetLayoutRef setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "Basic: DescSetLayout2";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::MESH_BIT_EXT },
		{1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT },
		{2, GraphicsAPI::IsD3D12() ? DescriptorType::D3D12_STRUCTURED_BUFFER : DescriptorType::STORAGE_BUFFER, 1, Shader::StageBit::MESH_BIT_EXT }, //D3D12_STRUCTURED_BUFFER as STORAGE_BUFFER implies UAV.
		{3, GraphicsAPI::IsD3D12() ? DescriptorType::D3D12_STRUCTURED_BUFFER : DescriptorType::STORAGE_BUFFER, 1, Shader::StageBit::MESH_BIT_EXT }	//D3D12_STRUCTURED_BUFFER as STORAGE_BUFFER implies UAV.
	};
	DescriptorSetLayoutRef setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "PostProcess: DescSetLayout3";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::INPUT_ATTACHMENT, 1, Shader::StageBit::FRAGMENT_BIT },
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
	descriptorSet_p0->AddBuffer(0, 0, { { ubViewCam } });
	descriptorSet_p1->AddBuffer(0, 0, { { ubViewMdl } });
	descriptorSet_p1->AddImage(0, 1, { { sampler, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descriptorSet_p1->AddBuffer(0, 2, { { vbv } });
	descriptorSet_p1->AddBuffer(0, 3, { { mbv } });
	descriptorSet_p0->Update();
	descriptorSet_p1->Update();
	descriptorSetCI.debugName = "PostProcess: Descriptor Set";
	descriptorSetCI.descriptorPool = descriptorPool;
	descriptorSetCI.descriptorSetLayouts = { setLayout3 };
	DescriptorSetRef descriptorSet1 = DescriptorSet::Create(&descriptorSetCI);
	descriptorSet1->AddImage(0, 0, { { nullptr, resolveAndInputImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descriptorSet1->Update();

	//Main RenderPass
	RenderPass::CreateInfo renderPassCI;
	renderPassCI.debugName = "Basic: RenderPass";
	renderPassCI.device = context->GetDevice();
	renderPassCI.attachments = {
		{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,						//Colour MSAA
		Image::SampleCountBit::SAMPLE_COUNT_8_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::STORE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::COLOUR_ATTACHMENT_OPTIMAL
		},
		{depthImage->GetCreateInfo().format,											//Depth MSAA
		Image::SampleCountBit::SAMPLE_COUNT_8_BIT,
		RenderPass::AttachmentLoadOp::CLEAR,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		},
		{swapchain->m_SwapchainImages[0]->GetCreateInfo().format,						//Resolve and input
		Image::SampleCountBit::SAMPLE_COUNT_1_BIT,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		RenderPass::AttachmentLoadOp::DONT_CARE,
		RenderPass::AttachmentStoreOp::DONT_CARE,
		Image::Layout::UNKNOWN,
		Image::Layout::SHADER_READ_ONLY_OPTIMAL
		},
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
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {{2, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {} },
		{PipelineType::GRAPHICS, {{2, Image::Layout::SHADER_READ_ONLY_OPTIMAL}}, {{3, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {}, {} }
	};
	renderPassCI.subpassDependencies = {
		{RenderPass::SubpassExternal, 0,
		PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT},
		{0, 1,
		PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
		(Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT},
	};
	RenderPassRef renderPass = RenderPass::Create(&renderPassCI);

	//Basic and PostProcessing pipelines
	Pipeline::CreateInfo pCI;
	pCI.debugName = "Basic";
	pCI.device = context->GetDevice();
	pCI.type = PipelineType::GRAPHICS;
	pCI.shaders = { meshShader, fragmentShader };
	pCI.vertexInputState.vertexInputBindingDescriptions = {}; //No Vertex Input State for Mesh shaders.
	pCI.vertexInputState.vertexInputAttributeDescriptions = {}; //No Vertex Input State for Mesh shaders.
	pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false }; //Input Topology should match mesh shader.
	pCI.tessellationState = {};
	pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
	pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
	pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_8_BIT, false, 1.0f, UINT32_MAX, false, false };
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
	PipelineRef pipeline = Pipeline::Create(&pCI);
	Pipeline::CreateInfo p1CI = pCI;
	p1CI.debugName = "PostProcess";
	p1CI.shaders = { postProcessVertexShader, postProcessFragmentShader };
	p1CI.vertexInputState.vertexInputBindingDescriptions = {};
	p1CI.vertexInputState.vertexInputAttributeDescriptions = {};
	p1CI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, UINT32_MAX, false, false };
	p1CI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	p1CI.depthStencilState = { false, false , CompareOp::NEVER, false, false, {}, {}, 0.0f, 1.0f };
	p1CI.layout = { {setLayout3 }, {} };
	p1CI.renderPass = renderPass;
	p1CI.subpassIndex = 1;
	PipelineRef postProcessPipeline = Pipeline::Create(&p1CI);

	Framebuffer::CreateInfo framebufferCI_0, framebufferCI_1;
	framebufferCI_0.debugName = "Framebuffer0";
	framebufferCI_0.device = context->GetDevice();
	framebufferCI_0.renderPass = renderPass;
	framebufferCI_0.attachments = { colourImageView, depthImageView, resolveAndInputImageView, swapchain->m_SwapchainImageViews[0] };
	framebufferCI_0.width = width;
	framebufferCI_0.height = height;
	framebufferCI_0.layers = 1;
	FramebufferRef framebuffer0 = Framebuffer::Create(&framebufferCI_0);
	framebufferCI_1.debugName = "Framebuffer1";
	framebufferCI_1.device = context->GetDevice();
	framebufferCI_1.renderPass = renderPass;
	framebufferCI_1.attachments = { colourImageView, depthImageView, resolveAndInputImageView, swapchain->m_SwapchainImageViews[1] };
	framebufferCI_1.width = width;
	framebufferCI_1.height = height;
	framebufferCI_1.layers = 1;
	FramebufferRef framebuffer1 = Framebuffer::Create(&framebufferCI_1);

	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "DrawFence";
	fenceCI.device = context->GetDevice();
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<FenceRef> draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
	Semaphore::CreateInfo acquireSemaphoreCI = { "AcquireSeamphore", context->GetDevice() };
	Semaphore::CreateInfo submitSemaphoreCI = { "SubmitSeamphore", context->GetDevice() };
	SemaphoreRef acquire = Semaphore::Create(&acquireSemaphoreCI);
	SemaphoreRef submit = Semaphore::Create(&submitSemaphoreCI);

	MIRU_CPU_PROFILE_END_SESSION();

	uint32_t frameIndex = 0;
	uint32_t frameCount = 0;
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

			meshShader->Recompile();
			fragmentShader->Recompile();
			pCI.shaders = { meshShader, fragmentShader };
			pipeline = Pipeline::Create(&pCI);

			postProcessVertexShader->Recompile();
			postProcessFragmentShader->Recompile();
			p1CI.shaders = { postProcessVertexShader, postProcessFragmentShader };
			postProcessPipeline = Pipeline::Create(&p1CI);

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			shaderRecompile = false;
			frameIndex = 0;

			//Transition any resource into the correct states.
			{
				cmdBuffer->Reset(2, false);
				cmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

				Barrier::CreateInfo bCI;
				bCI.type = Barrier::Type::IMAGE;
				bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
				bCI.dstAccess = Barrier::AccessBit::NONE_BIT;
				bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
				bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
				bCI.image = resolveAndInputImage;
				bCI.oldLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
				bCI.newLayout = Image::Layout::UNKNOWN;
				bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
				BarrierRef b = Barrier::Create(&bCI);
				cmdBuffer->PipelineBarrier(2, PipelineStageBit::BOTTOM_OF_PIPE_BIT, PipelineStageBit::TOP_OF_PIPE_BIT, DependencyBit::NONE_BIT, { b });

				cmdBuffer->End(2);
			}
			CommandBuffer::SubmitInfo si = { { 2 }, {}, {}, {}, {}, {}, };
			cmdBuffer->Submit({ si }, nullptr);
		}
		if (swapchain->m_Resized || windowResize)
		{
			swapchain->Resize(width, height);

			pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
			pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
			pipeline = Pipeline::Create(&pCI);

			p1CI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
			p1CI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
			postProcessPipeline = Pipeline::Create(&p1CI);

			colourCI.width = width;
			colourCI.height = height;
			colourImage = Image::Create(&colourCI);
			colourImageViewCI.image = colourImage;
			colourImageView = ImageView::Create(&colourImageViewCI);

			depthCI.width = width;
			depthCI.height = height;
			depthImage = Image::Create(&depthCI);
			depthImageViewCI.image = depthImage;
			depthImageView = ImageView::Create(&depthImageViewCI);

			resolveAndInputImageCI.width = width;
			resolveAndInputImageCI.height = height;
			resolveAndInputImage = Image::Create(&resolveAndInputImageCI);
			resolveAndInputImageViewCI.image = resolveAndInputImage;
			resolveAndInputImageView = ImageView::Create(&resolveAndInputImageViewCI);

			descriptorSet1 = nullptr;
			descriptorSet1 = DescriptorSet::Create(&descriptorSetCI);
			descriptorSet1->AddImage(0, 0, { { nullptr, resolveAndInputImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			descriptorSet1->Update();

			framebufferCI_0.attachments = { colourImageView, depthImageView, resolveAndInputImageView, swapchain->m_SwapchainImageViews[0] };
			framebufferCI_0.width = width;
			framebufferCI_0.height = height;
			framebuffer0 = Framebuffer::Create(&framebufferCI_0);

			framebufferCI_1.attachments = { colourImageView, depthImageView, resolveAndInputImageView, swapchain->m_SwapchainImageViews[1] };
			framebufferCI_1.width = width;
			framebufferCI_1.height = height;
			framebuffer1 = Framebuffer::Create(&framebufferCI_1);

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquire = Semaphore::Create(&acquireSemaphoreCI);
			submit = Semaphore::Create(&submitSemaphoreCI);

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

			swapchain->AcquireNextImage(acquire, frameIndex);

			draws[frameIndex]->Wait();
			draws[frameIndex]->Reset();

			cmdBuffer->Reset(frameIndex, false);
			cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
			cmdBuffer->BeginRenderPass(frameIndex, frameIndex == 0 ? framebuffer0 : framebuffer1, { {r, g, b, 1.0f}, {0.0f, 0}, {r, g, b, 1.0f}, {r, g, b, 1.0f} });
			cmdBuffer->BindPipeline(frameIndex, pipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet_p0 }, 0, pipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet_p1 }, 1, pipeline);
			cmdBuffer->DrawMeshTasks(frameIndex, meshlets.size(), 1, 1);

			cmdBuffer->NextSubpass(frameIndex);
			cmdBuffer->BindPipeline(frameIndex, postProcessPipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet1 }, 0, postProcessPipeline);
			cmdBuffer->Draw(frameIndex, 3);
			cmdBuffer->EndRenderPass(frameIndex);
			cmdBuffer->End(frameIndex);

			CommandBuffer::SubmitInfo mainSI = { { frameIndex }, { acquire }, {}, { base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT }, { submit }, {} };
			cmdBuffer->Submit({ mainSI }, draws[frameIndex]);

			swapchain->Present(cmdPool, submit, frameIndex);

			proj = Mat4::Perspective(3.14159 / 2.0, float(width) / float(height), 0.1f, 100.0f);
			if (GraphicsAPI::IsVulkan())
				proj.f *= -1;
			modl = Mat4::Translation({ 0.0f, 0.0f, -1.5f })
				//* translate(mat4(1.0f), { float(var_x)/10.0f, float(var_y)/10.0f, float(var_z)/10.0f})
				* Mat4::Rotation((var_x * 5.0f) * 3.14159 / 180.0, { 0, 1, 0 })
				* Mat4::Rotation((var_y * 5.0f) * 3.14159 / 180.0, { 1, 0, 0 })
				* Mat4::Rotation((var_z * 5.0f) * 3.14159 / 180.0, { 0, 0, 1 });

			memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
			memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

			cpu_alloc_0->SubmitData(ub1->GetAllocation(), 0, 2 * sizeof(Mat4), ubData);
			cpu_alloc_0->SubmitData(ub2->GetAllocation(), 0, sizeof(Mat4), (void*)&modl.a);

			frameCount++;
		}
	}
	context->DeviceWaitIdle();
}