#include "../../../MARS/MARS/src/mars.h"
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
int var_x, var_y, var_z = 0;
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
	if (msg == WM_KEYDOWN && wparam == 0x49) //I
		var_y++;
	if (msg == WM_KEYDOWN && wparam == 0x4B) //K
		var_y--;
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
	
	MIRU_CPU_PROFILE_BEGIN_SESSION("miru_profile_result.txt");

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

	float vertices[24] =
	{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
		+0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
		+0.5f, +0.5f, 0.0f, 1.0f, 1.0f, 0.0f,
		-0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
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
	imageCI.debugName = "Image";
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
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		Ref<Barrier> b = Barrier::Create(&bCI);
		cmdCopyBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, { b });
		cmdCopyBuffer->CopyBufferToImage(0, c_imageBuffer, image, Image::Layout::TRANSFER_DST_OPTIMAL, { {0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}} });
		
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
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
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
	vbViewCI.stride = 6 * sizeof(float);
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
	imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
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

	mars::Mat4 proj = mars::Mat4::Identity();
	mars::Mat4 view = mars::Mat4::Identity();
	mars::Mat4 modl = mars::Mat4::Identity();

	float ubData[2 * sizeof(mars::Mat4)];
	memcpy(ubData + 0 * 16,	&proj.a, sizeof(mars::Mat4));
	memcpy(ubData + 1 * 16,	&view.a, sizeof(mars::Mat4));

	Buffer::CreateInfo ubCI;
	ubCI.debugName = "CameraUB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM;
	ubCI.size = 2 * sizeof(mars::Mat4);
	ubCI.data = ubData;
	ubCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> ub1 = Buffer::Create(&ubCI);
	ubCI.debugName = "ModelUB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM;
	ubCI.size = sizeof(mars::Mat4);
	ubCI.data = &modl.a;
	ubCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> ub2 = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubViewCamCI;
	ubViewCamCI.debugName = "CameraUBView";
	ubViewCamCI.device = context->GetDevice();
	ubViewCamCI.type = BufferView::Type::UNIFORM;
	ubViewCamCI.pBuffer = ub1;
	ubViewCamCI.offset = 0;
	ubViewCamCI.size = 2 * sizeof(mars::Mat4);
	ubViewCamCI.stride = 0;
	Ref<BufferView> ubViewCam = BufferView::Create(&ubViewCamCI);
	BufferView::CreateInfo ubViewMdlCI;
	ubViewMdlCI.debugName = "ModelUBView";
	ubViewMdlCI.device = context->GetDevice();
	ubViewMdlCI.type = BufferView::Type::UNIFORM;
	ubViewMdlCI.pBuffer = ub2;
	ubViewMdlCI.offset = 0;
	ubViewMdlCI.size = sizeof(mars::Mat4);
	ubViewMdlCI.stride = 0;
	Ref<BufferView> ubViewMdl = BufferView::Create(&ubViewMdlCI);

	Image::CreateInfo depthCI;
	depthCI.debugName = "DepthImage";
	depthCI.device = context->GetDevice();
	depthCI.type = Image::Type::TYPE_2D;
	depthCI.format = Image::Format::D32_SFLOAT;
	depthCI.width = width;
	depthCI.height = height;
	depthCI.depth = 1;
	depthCI.mipLevels = 1;
	depthCI.arrayLayers = 1;
	depthCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	depthCI.usage = Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	depthCI.layout = api.IsD3D12() ? Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL : Image::Layout::UNKNOWN;
	depthCI.size = 0;
	depthCI.data = nullptr;
	depthCI.pMemoryBlock = gpu_mb_0;
	Ref<Image> depthImage = Image::Create(&depthCI);
	
	ImageView::CreateInfo depthImageViewCI;
	depthImageViewCI.debugName = "DepthImageView";
	depthImageViewCI.device = context->GetDevice();
	depthImageViewCI.pImage = depthImage;
	depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
	Ref<ImageView> depthImageView = ImageView::Create(&depthImageViewCI);

	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "Image Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1},  {DescriptorType::UNIFORM_BUFFER, 2} };
	descriptorPoolCI.maxSets = 2;
	Ref<DescriptorPool> descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "Basic Shader DescSetLayout";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER,  1, Shader::StageBit::VERTEX_BIT } };
	Ref<DescriptorSetLayout> setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.descriptorSetLayoutBinding = { 
		{1, DescriptorType::COMBINED_IMAGE_SAMPLER,  1, Shader::StageBit::FRAGMENT_BIT }, 
		{0, DescriptorType::UNIFORM_BUFFER,  1, Shader::StageBit::VERTEX_BIT }
	};
	Ref<DescriptorSetLayout> setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetCI;
	descriptorSetCI.debugName = "Image Descriptor Set";
	descriptorSetCI.pDescriptorPool = descriptorPool;
	descriptorSetCI.pDescriptorSetLayouts = {setLayout1, setLayout2 };
	Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(&descriptorSetCI);
	descriptorSet->AddBuffer(0, 0, { { ubViewCam } });
	descriptorSet->AddBuffer(1, 0, { { ubViewMdl } });
	descriptorSet->AddImage(1, 1, { { sampler, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
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
		},
		{depthImage->GetCreateInfo().format,
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
		{PipelineType::GRAPHICS, {}, {{0, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL}}, {}, {{1, Image::Layout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL}}, {} }
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
	pCI.vertexInputState.vertexInputBindingDescriptions = { {0, sizeof(vertices)/4, VertexInputRate::VERTEX} };
	pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0, "POSITION"}, {1, 0, VertexType::VEC2, 16, "TEXCOORD"} };
	pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
	pCI.tessellationState = {};
	pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
	pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
	pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
	pCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false };
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
	pCI.layout = { {setLayout1, setLayout2}, {} };
	pCI.renderPass = renderPass;
	pCI.subpassIndex = 0;
	Ref<Pipeline> pipeline = Pipeline::Create(&pCI);

	Framebuffer::CreateInfo framebufferCI_0, framebufferCI_1;
	framebufferCI_0.debugName = "Framebuffer0";
	framebufferCI_0.device = context->GetDevice();
	framebufferCI_0.renderPass = renderPass;
	framebufferCI_0.attachments = { swapchain->m_SwapchainImageViews[0], depthImageView };
	framebufferCI_0.width = width;
	framebufferCI_0.height = height;
	framebufferCI_0.layers = 1;
	Ref<Framebuffer> framebuffer0 = Framebuffer::Create(&framebufferCI_0);
	framebufferCI_1.debugName = "Framebuffer1";
	framebufferCI_1.device = context->GetDevice();
	framebufferCI_1.renderPass = renderPass;
	framebufferCI_1.attachments = { swapchain->m_SwapchainImageViews[1], depthImageView };
	framebufferCI_1.width = width;
	framebufferCI_1.height = height;
	framebufferCI_1.layers = 1;
	Ref<Framebuffer> framebuffer1 = Framebuffer::Create(&framebufferCI_1);

	auto RecordPresentCmdBuffers = [&]()
	{	
		for (uint32_t i = 0; i < cmdBuffer->GetCreateInfo().commandBufferCount - 1; i++)
		{
			cmdBuffer->Begin(i, CommandBuffer::UsageBit::SIMULTANEOUS);
			cmdBuffer->BeginRenderPass(i, i == 0 ? framebuffer0 : framebuffer1, { {0.5f, 0.5f, 0.5f, 1.0f}, {0.0f, 0} });
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

	MIRU_CPU_PROFILE_END_SESSION();

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

			framebufferCI_0.attachments = { swapchain->m_SwapchainImageViews[0], depthImageView };
			framebufferCI_0.width = width;
			framebufferCI_0.height = height;
			framebuffer0 = Framebuffer::Create(&framebufferCI_0);

			framebufferCI_1.attachments = { swapchain->m_SwapchainImageViews[1], depthImageView };
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
		{
			proj = mars::Mat4::Perspective(90.0f, float(width) / float(height), 0.1f, 100.0f);
			if(api.IsVulkan())
				proj.f *= -1;
			modl = mars::Mat4::Translation({(float)var_x / 10.0f, (float)var_y / 10.0f, -1.0f + (float)var_z / 10.0f });
			memcpy(ubData + 0 * 16, &proj.a, sizeof(mars::Mat4));
			memcpy(ubData + 1 * 16, &view.a, sizeof(mars::Mat4));

			cpu_mb_0->SubmitData(ub1->GetResource(), 2 * sizeof(mars::Mat4), ubData);
			cpu_mb_0->SubmitData(ub2->GetResource(), 2 * sizeof(mars::Mat4), &modl.a);
		}
		cmdBuffer->Present({ 0, 1 }, swapchain, draws, acquire, submit, windowResize);
		frameIndex++;
	}
	context->DeviceWaitIdle();
}