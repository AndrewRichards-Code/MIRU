#include "miru_core.h"
#include "vulkan/VKImage.h"
#include "directx12/D3D12Image.h"
#include "vulkan/VKSwapchain.h"
#include "directx12/D3D12Swapchain.h"

using namespace miru;
using namespace crossplatform;
#if 1
bool g_WindowQuit = false;
LRESULT CALLBACK WindProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY || msg == WM_CLOSE)
	{
		PostQuitMessage(0);
		g_WindowQuit = true;
		return 0;
	}

	return DefWindowProc(handle, msg, wparam, lparam);
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

	uint32_t width = 800;
	uint32_t height = 600;
	HWND window = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, 100, 100, width, height, 0, 0, 0, 0);
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
	mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
	Ref<MemoryBlock> cpu_mb_0 = MemoryBlock::Create(&mbCI);

	float data[16] =
	{
		0.00f, 0.00f, 0.00f, 0.00f,
		0.33f, 0.33f, 0.33f, 0.33f,
		0.66f, 0.66f, 0.66f, 0.66f,
		1.00f, 1.00f, 1.00f, 1.00f
	};
	Image::CreateInfo texture1CI;
	texture1CI.debugName = "Texture1";
	texture1CI.device = context->GetDevice();
	texture1CI.type = Image::Type::TYPE_2D;
	texture1CI.format = Image::Format::B8G8R8A8_UNORM;
	texture1CI.width = 2;
	texture1CI.height = 2; 
	texture1CI.depth = 1;
	texture1CI.mipLevels = 1;
	texture1CI.arrayLayers = 1;
	texture1CI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texture1CI.usage = Image::UsageBit::SAMPLED_BIT;
	texture1CI.layout = Image::Layout::UNKNOWN;
	texture1CI.size = sizeof(data);
	texture1CI.data = data;
	texture1CI.pMemoryBlock = cpu_mb_0;
	Ref<Image> texture1 = Image::Create(&texture1CI);

	ImageView::CreateInfo texture1viewCI;
	texture1viewCI.debugName = "Texture1View";
	texture1viewCI.device = context->GetDevice();
	texture1viewCI.pImage = texture1;
	texture1viewCI.subresourceRange = { Image::AspectBit::COLOR_BIT, 0, 1, 0, 1 };
	Ref<ImageView> texture1view = ImageView::Create(&texture1viewCI);

	DescriptorPool::CreateInfo descPoolCI;
	descPoolCI.debugName = "DescriptorPool1";
	descPoolCI.device = context->GetDevice();
	descPoolCI.poolSizes = { {DescriptorType::SAMPLED_IMAGE, 1} };
	descPoolCI.maxSets = 1;
	Ref<DescriptorPool> descPool = DescriptorPool::Create(&descPoolCI);

	DescriptorSetLayout::CreateInfo descSetLayoutCI;
	descSetLayoutCI.debugName = "DescriptorSetLayout1";
	descSetLayoutCI.device = context->GetDevice();
	descSetLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::SAMPLED_IMAGE, 1, ShaderStageBit::ALL_GRAPHICS} };
	Ref<DescriptorSetLayout> descSetLayout = DescriptorSetLayout::Create(&descSetLayoutCI);

	DescriptorSet::CreateInfo descSetCI;
	descSetCI.debugName = "DescriptorSetLayout1";
	descSetCI.pDescriptorPool = descPool;
	descSetCI.pDescriptorSetLayouts = { descSetLayout };
	Ref<DescriptorSet> descSet = DescriptorSet::Create(&descSetCI);
	descSet->AddImage(0, 0, { { nullptr, texture1view, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
	descSet->Update();

	Image::CreateInfo swapchainTexCI;
	swapchainTexCI.debugName = "";
	swapchainTexCI.device = context->GetDevice();
	swapchainTexCI.type = Image::Type::TYPE_2D;
	swapchainTexCI.format = Image::Format::B8G8R8A8_UNORM;
	swapchainTexCI.width = swapchainTexCI.height = swapchainTexCI.depth = 1;
	swapchainTexCI.mipLevels = 1;
	swapchainTexCI.arrayLayers = 1;
	swapchainTexCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	swapchainTexCI.usage = Image::UsageBit::TRANSFER_DST_BIT;
	swapchainTexCI.layout = Image::Layout::UNKNOWN;
	swapchainTexCI.size = 0;
	swapchainTexCI.data = nullptr;
	swapchainTexCI.pMemoryBlock = nullptr;
	Ref<Image> swapchainTex = Image::Create(&swapchainTexCI);

	uint64_t oldTextureID = 0;
	if(api.GetAPI() == GraphicsAPI::API::D3D12)
		oldTextureID = (uint64_t)std::dynamic_pointer_cast<d3d12::Image>(swapchainTex)->m_Image;
	else
		oldTextureID = (uint64_t)std::dynamic_pointer_cast<vulkan::Image>(swapchainTex)->m_Image;

	for (uint32_t i = 0; i < cmdBuffer->GetCreateInfo().commandBufferCount; i++)
	{
		cmdBuffer->Begin(i, CommandBuffer::UsageBit::SIMULTANEOUS);

		if (api.GetAPI() == GraphicsAPI::API::D3D12)
		{
			std::dynamic_pointer_cast<d3d12::Image>(swapchainTex)->m_Image = reinterpret_cast<d3d12::Swapchain*>(swapchain.get())->m_SwapchainRTVs[i];
			std::dynamic_pointer_cast<d3d12::Image>(swapchainTex)->m_CurrentResourceState = D3D12_RESOURCE_STATE_COMMON;
		}
		else
		{
			std::dynamic_pointer_cast<vulkan::Image>(swapchainTex)->m_Image = reinterpret_cast<vulkan::Swapchain*>(swapchain.get())->m_SwapchainImages[i];
			std::dynamic_pointer_cast<vulkan::Image>(swapchainTex)->m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		crossplatform::Image::ClearColourValue clear = { 1.0f, 0.0f, 0.0f, 1.0f };
		crossplatform::Image::SubresourceRange subres = { Image::AspectBit::COLOR_BIT, 0, 1, 0, 1 };

		Barrier::CreateInfo b1CI;
		b1CI.type = Barrier::Type::IMAGE;
		b1CI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
		b1CI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		b1CI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		b1CI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		b1CI.pImage = swapchainTex;
		b1CI.oldLayout = Image::Layout::UNKNOWN;
		b1CI.newLayout = api.GetAPI() == GraphicsAPI::API::D3D12 ? Image::Layout::COLOR_ATTACHMENT_OPTIMAL : Image::Layout::TRANSFER_DST_OPTIMAL;
		b1CI.subresoureRange = subres;
		Ref<Barrier> b1 = Barrier::Create(&b1CI);

		Barrier::CreateInfo b2CI;
		b2CI.type = Barrier::Type::IMAGE;
		b2CI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		b2CI.dstAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
		b2CI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		b2CI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		b2CI.pImage = swapchainTex;
		b2CI.oldLayout = api.GetAPI() == GraphicsAPI::API::D3D12 ? Image::Layout::COLOR_ATTACHMENT_OPTIMAL : Image::Layout::TRANSFER_DST_OPTIMAL;
		b2CI.newLayout = Image::Layout::PRESENT_SRC;
		b2CI.subresoureRange = subres;
		Ref<Barrier> b2 = Barrier::Create(&b2CI);
		
		cmdBuffer->PipelineBarrier(i, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::TRANSFER_BIT, { b1 });
		cmdBuffer->ClearColourImage(i, swapchainTex, crossplatform::Image::Layout::TRANSFER_DST_OPTIMAL, clear, { subres });
		cmdBuffer->PipelineBarrier(i, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::BOTTOM_OF_PIPE_BIT, { b2 });
		cmdBuffer->End(i);
	}

	if (api.GetAPI() == GraphicsAPI::API::D3D12)
		std::dynamic_pointer_cast<d3d12::Image>(swapchainTex)->m_Image = (ID3D12Resource*)(void*)oldTextureID;
	else
		std::dynamic_pointer_cast<vulkan::Image>(swapchainTex)->m_Image = (VkImage)oldTextureID;

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
		//Check for WindowMessages
		if (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		cmdBuffer->Present({ 0, 1 }, swapchain, draws, acquire, submit);
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