#include "miru_core.h"

using namespace miru;
using namespace crossplatform;
#if 1
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
	//api.LoadRenderDoc();
	api.SetUseSetName();
	//api.SetAPI(GraphicsAPI::API::D3D12);
	api.SetAPI(GraphicsAPI::API::VULKAN);

	Context::CreateInfo contextCI;
	contextCI.api_version_major = api.GetAPI() == GraphicsAPI::API::D3D12 ? 11 : 1;
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
	shaderCI.filepath = "../MIRU_SHADER_COMPILER/res/bin/basic.vert.spv";
	shaderCI.entryPoint = "main";
	Ref<Shader> vertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Basic_Fragment";
	shaderCI.stage = Shader::StageBit::PIXEL_BIT;
	shaderCI.filepath = "../MIRU_SHADER_COMPILER/res/bin/basic.frag.spv";
	Ref<Shader> fragmentShader = Shader::Create(&shaderCI);

	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CmdPool";
	cmdPoolCI.pContext = context;
	cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	cmdPoolCI.queueFamilyIndex = 0;
	Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);

	CommandBuffer::CreateInfo cmdBufferCI;
	cmdBufferCI.debugName = "CmdBuffer";
	cmdBufferCI.pCommandPool = cmdPool;
	cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdBufferCI.commandBufferCount = 2;
	Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

	MemoryBlock::CreateInfo mbCI;
	mbCI.debugName = "CPU_MB_0";
	mbCI.pContext = context;
	mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_1MB;
	mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT | MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
	Ref<MemoryBlock> cpu_mb_0 = MemoryBlock::Create(&mbCI);

	float vertices[16] =
	{
		-0.5f, -0.5f, 0.0f, 1.0f,
		+0.5f, -0.5f, 0.0f, 1.0f,
		+0.5f, +0.5f, 0.0f, 1.0f,
		-0.5f, +0.5f, 0.0f, 1.0f
	};
	uint32_t indices[6] = { 0,1,2,2,3,0 };

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices";
	verticesBufferCI.device = context->GetDevice();
	verticesBufferCI.usage = Buffer::UsageBit::VERTEX | Buffer::UsageBit::TRANSFER_SRC;
	verticesBufferCI.size = sizeof(vertices);
	verticesBufferCI.data = vertices;
	verticesBufferCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> vb = Buffer::Create(&verticesBufferCI);

	Buffer::CreateInfo indicesBufferCI;
	indicesBufferCI.debugName = "Indices";
	indicesBufferCI.device = context->GetDevice();
	indicesBufferCI.usage = Buffer::UsageBit::INDEX | Buffer::UsageBit::TRANSFER_SRC;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.pMemoryBlock = cpu_mb_0;
	Ref<Buffer> ib = Buffer::Create(&indicesBufferCI);

	BufferView::CreateInfo vbViewCI;
	vbViewCI.debugName = "VerticesBufferView";
	vbViewCI.device = context->GetDevice();
	vbViewCI.type = BufferView::Type::VERTEX;
	vbViewCI.pBuffer = vb;
	vbViewCI.offset = 0;
	vbViewCI.size = 16 * sizeof(float);
	vbViewCI.stride = 4 * sizeof(float);
	Ref<BufferView> vbv = BufferView::Create(&vbViewCI);

	BufferView::CreateInfo ibViewCI;
	ibViewCI.debugName = "VerticesBufferView";
	ibViewCI.device = context->GetDevice();
	ibViewCI.type = BufferView::Type::INDEX;
	ibViewCI.pBuffer = ib;
	ibViewCI.offset = 0;
	ibViewCI.size = 6 * sizeof(uint32_t);
	ibViewCI.stride = sizeof(uint32_t);
	Ref<BufferView> ibv = BufferView::Create(&ibViewCI);

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
		Image::Layout::UNKNOWN,
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
	pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0} };
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
	pCI.layout = { {}, {} };
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
		cmdPool->Reset(false);
		for (uint32_t i = 0; i < cmdBuffer->GetCreateInfo().commandBufferCount; i++)
		{
			cmdBuffer->Begin(i, CommandBuffer::UsageBit::SIMULTANEOUS);
			cmdBuffer->BeginRenderPass(i, i == 0 ? framebuffer0 : framebuffer1, { {1.0f, 0.0f, 0.0f, 1.0f} });
			cmdBuffer->BindPipeline(i, pipeline);
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
	std::vector<Ref<Semaphore>>submit = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI)};

	uint32_t frameIndex = 0;
	MSG msg = { 0 };
	//Main Render Loop
	while (!g_WindowQuit)
	{
		WindowUpdate();

		if (shaderRecompile)
		{
			vkDeviceWaitIdle(*(VkDevice*)context->GetDevice());

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
	vkDeviceWaitIdle(*(VkDevice*)context->GetDevice());
}
#endif 
#if 0
void CreateHeap(ID3D12Device* device, ID3D12Heap* heap)
{
	D3D12_HEAP_DESC heapDesc;
	heapDesc.SizeInBytes = 1048576 * 256;										//Size of heap
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_CUSTOM;							//UPLOAD = CPU optimised, DEFAULT = GPU optimised
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	//If UsageBit == CUSTOM, CPU write access, else set to 0;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;			//If UsageBit == CUSTOM, L0 = CPU, L1 = GPU, else set to 0;
	heapDesc.Properties.CreationNodeMask = 0;									//Which device store the heap
	heapDesc.Properties.VisibleNodeMask = 0;									//Which devices can see the heap
	heapDesc.Alignment = 0;
	heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
	HRESULT result = device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap));
};

int main()
{
	ID3D12Debug* m_Debug;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug));
	m_Debug->EnableDebugLayer();

	IDXGIFactory4* m_Factory;
	CreateDXGIFactory2(0, IID_PPV_ARGS(&m_Factory));

	std::vector<IDXGIAdapter4*> m_Adapters;
	UINT i = 0;
	IDXGIAdapter1* adapter;
	while (m_Factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		m_Adapters.push_back(reinterpret_cast<IDXGIAdapter4*>(adapter));
		i++;
	}

	ID3D12Device* device = nullptr;
	result = D3D12CreateDevice(m_Adapters[0], D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device));

	ID3D12Heap* heap = nullptr;
	std::thread createHeap(CreateHeap, device, heap);

	ID3D12Resource* res = nullptr;
	UINT offset = 0;
	D3D12_RESOURCE_DESC resDesc;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		//General Type of Resource
	resDesc.Alignment = 0;
	resDesc.Width = 4 * sizeof(float);							//Alias for bufferSize
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;						//Required format for buffers
	resDesc.SampleDesc = {1, 0};								//Required sampleDesc for buffers
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			//Required layout for buffers
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;					//How the resource is to be used
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;	//Specific type of the resource
	D3D12_CLEAR_VALUE* clear = 0;								//Clear values for textures
	
	createHeap.join();
	if(heap)
		result = device->CreatePlacedResource(heap, offset, &resDesc, state, clear, IID_PPV_ARGS(&res));


	if(res)
		res->Release();
	if(heap)
		heap->Release();
	if(device)
		device->Release();
}
#endif