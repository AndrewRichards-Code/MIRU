#if defined(_WIN64)
#include "miru_core.h"
#include "maths.h"

#include "STBI/stb_image.h"

using namespace miru;
using namespace base;

static HWND window;
static bool g_WindowQuit = false;
static uint32_t width = 800;
static uint32_t height = 600;
static bool windowResize = false;
static bool shaderRecompile = false;
static int var_x, var_y, var_z = 0;

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

void Raytracing()
{
	//GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
	GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
	GraphicsAPI::AllowSetName();
	GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::NONE);

	MIRU_CPU_PROFILE_BEGIN_SESSION("miru_profile_result.txt");

	Context::CreateInfo contextCI;
	contextCI.applicationName = "MIRU_TEST";
	contextCI.extensions = Context::ExtensionsBit::RAY_TRACING;
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

	//Ray Tracing library
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "RayTracing: Library Shader Module";
	shaderCI.device = context->GetDevice();
	shaderCI.stageAndEntryPoints = {
		{ Shader::StageBit::RAYGEN_BIT, "ray_generation_main"},
		{ Shader::StageBit::ANY_HIT_BIT, "any_hit_main"},
		{ Shader::StageBit::CLOSEST_HIT_BIT, "closest_hit_main"},
		{ Shader::StageBit::MISS_BIT, "miss_main"},
	};
	shaderCI.binaryFilepath = "res/bin/raytracing_lib_6_3.spv";
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments = {
		"res/shaders/raytracing.hlsl",
		"res/bin",
		{"../MIRU_SHADER_COMPILER/shaders/includes"},
		"",
		"lib_6_3",
		{},
		true,
		true,
		{"-Zi", "-Od", "-Fd"},
		""
	};
	ShaderRef raytracingShader = Shader::Create(&shaderCI);

	//CmdPool and CmdBuffer
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

	//Allocator
	Allocator::CreateInfo allocCI;
	allocCI.debugName = "CPU_ALLOC_0";
	allocCI.context = context;
	allocCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
	allocCI.properties = Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT;
	AllocatorRef cpu_alloc_0 = Allocator::Create(&allocCI);
	allocCI.debugName = "GPU_ALLOC_0";
	allocCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	AllocatorRef gpu_alloc_0 = Allocator::Create(&allocCI);

	//Geometry
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

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices Buffer";
	verticesBufferCI.device = context->GetDevice();
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
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
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_ib = Buffer::Create(&indicesBufferCI);
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	indicesBufferCI.data = nullptr;
	indicesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_ib = Buffer::Create(&indicesBufferCI);

	//Uniform buffers
	Mat4 proj = Mat4::Perspective(3.14159 / 2.0, float(width) / float(height), 0.1f, 100.0f);
	Mat4 view = Mat4::Identity();
	Mat4 modl = Mat4::Translation({ 0.0f, 0.0f, -1.5f });

	float ubData[2 * sizeof(Mat4)];
	memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
	memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

	Buffer::CreateInfo ubCI;
	ubCI.debugName = "Model UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	ubCI.size = sizeof(Mat4);
	ubCI.data = &modl.a;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ub1 = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubViewMdlCI;
	ubViewMdlCI.debugName = "Model UBView";
	ubViewMdlCI.device = context->GetDevice();
	ubViewMdlCI.type = BufferView::Type::UNIFORM;
	ubViewMdlCI.buffer = ub1;
	ubViewMdlCI.offset = 0;
	ubViewMdlCI.size = sizeof(Mat4);
	ubViewMdlCI.stride = 0;
	BufferViewRef ubViewMdl = BufferView::Create(&ubViewMdlCI);

	struct Camera
	{
		Mat4 viewMatrix;
		Vec3 position;
		float aspectRatio;
		Vec4 direction;
	};
	struct SceneConstants
	{
		Camera camera;
		float TMin;
		float TMax;
	};
	SceneConstants ubSceneConstantsData = { {view, {0, 0, 0}, float(width) / float(height), {0, 0, -1.5, 0}}, 0.1f, 100.0f };
	ubCI.debugName = "SceneConstants UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = sizeof(SceneConstants);
	ubCI.data = &ubSceneConstantsData;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ubSceneConstants = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubSceneConstantsCI;
	ubSceneConstantsCI.debugName = "RTShaderConstants UBView";
	ubSceneConstantsCI.device = context->GetDevice();
	ubSceneConstantsCI.type = BufferView::Type::UNIFORM;
	ubSceneConstantsCI.buffer = ubSceneConstants;
	ubSceneConstantsCI.offset = 0;
	ubSceneConstantsCI.size = sizeof(SceneConstants);
	ubSceneConstantsCI.stride = 0;
	BufferViewRef ubViewSceneConstants = BufferView::Create(&ubSceneConstantsCI);

	//RW Image
	Image::CreateInfo RT_RWImageCI;
	RT_RWImageCI.debugName = "RT_RWImage";
	RT_RWImageCI.device = context->GetDevice();
	RT_RWImageCI.type = Image::Type::TYPE_2D;
	RT_RWImageCI.format = swapchain->m_SwapchainImages[0]->GetCreateInfo().format;
	RT_RWImageCI.width = width;
	RT_RWImageCI.height = height;
	RT_RWImageCI.depth = 1;
	RT_RWImageCI.mipLevels = 1;
	RT_RWImageCI.arrayLayers = 1;
	RT_RWImageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	RT_RWImageCI.usage = Image::UsageBit::STORAGE_BIT | Image::UsageBit::TRANSFER_SRC_BIT;
	RT_RWImageCI.layout = Image::Layout::UNKNOWN;
	RT_RWImageCI.size = width * height * 4;
	RT_RWImageCI.data = nullptr;
	RT_RWImageCI.allocator = gpu_alloc_0;
	RT_RWImageCI.externalImage = nullptr;
	ImageRef RT_RWImage = Image::Create(&RT_RWImageCI);

	ImageView::CreateInfo RT_RWImageViewCI;
	RT_RWImageViewCI.debugName = "RT_RWImageView";
	RT_RWImageViewCI.device = context->GetDevice();
	RT_RWImageViewCI.image = RT_RWImage;
	RT_RWImageViewCI.viewType = Image::Type::TYPE_2D;
	RT_RWImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	ImageViewRef RT_RWImageView = ImageView::Create(&RT_RWImageViewCI);

	//Acceleration structure building
	//BLAS
	AccelerationStructureBuildInfo::BuildGeometryInfo asbiGBI;
	asbiGBI.device = context->GetDevice();
	asbiGBI.type = AccelerationStructureBuildInfo::BuildGeometryInfo::Type::BOTTOM_LEVEL;
	asbiGBI.flags = AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::PREFER_FAST_TRACE_BIT;
	asbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::BUILD;
	asbiGBI.srcAccelerationStructure = nullptr;
	asbiGBI.dstAccelerationStructure = nullptr;
	asbiGBI.geometries.clear();
	asbiGBI.geometries.push_back({});
	asbiGBI.geometries[0].type = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::Type::TRIANGLES;
	asbiGBI.geometries[0].triangles = {
				VertexType::VEC3,
				GetBufferDeviceAddress(context->GetDevice(), c_vb),
				static_cast<uint64_t>(4 * sizeof(float)),
				_countof(vertices) / 4,
				IndexType::UINT32,
				GetBufferDeviceAddress(context->GetDevice(), c_ib),
				_countof(indices),
				GetBufferDeviceAddress(context->GetDevice(), ub1)
	};
	asbiGBI.geometries[0].flags = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::FlagBit::OPAQUE_BIT;
	asbiGBI.scratchData = DeviceOrHostAddressNull;
	asbiGBI.buildType = AccelerationStructureBuildInfo::BuildGeometryInfo::BuildType::DEVICE;
	asbiGBI.maxPrimitiveCounts = _countof(indices) / 3;
	AccelerationStructureBuildInfoRef blas_asbi = AccelerationStructureBuildInfo::Create(&asbiGBI);

	Buffer::CreateInfo asBufferCI;
	asBufferCI.debugName = "BLASBuffer";
	asBufferCI.device = context->GetDevice();
	asBufferCI.usage = Buffer::UsageBit::ACCELERATION_STRUCTURE_STORAGE_BIT;
	asBufferCI.size = blas_asbi->GetBuildSizesInfo().accelerationStructureSize;
	asBufferCI.data = nullptr;
	asBufferCI.allocator = gpu_alloc_0;
	BufferRef asBuffer_BLAS = Buffer::Create(&asBufferCI);

	Buffer::CreateInfo scratchBufferCI;
	scratchBufferCI.debugName = "BLASScratchBuffer";
	scratchBufferCI.device = context->GetDevice();
	scratchBufferCI.usage = Buffer::UsageBit::STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	scratchBufferCI.size = blas_asbi->GetBuildSizesInfo().buildScratchSize;
	scratchBufferCI.data = nullptr;
	scratchBufferCI.allocator = gpu_alloc_0;
	BufferRef scratchBuffer_BLAS = Buffer::Create(&scratchBufferCI);

	AccelerationStructure::CreateInfo asCI;
	asCI.debugName = "BLAS";
	asCI.device = context->GetDevice();
	asCI.flags = AccelerationStructure::FlagBit::NONE_BIT;
	asCI.buffer = asBuffer_BLAS;
	asCI.offset = 0;
	asCI.size = asBufferCI.size;
	asCI.type = AccelerationStructure::Type::BOTTOM_LEVEL;
	asCI.deviceAddress = DeviceAddressNull;
	AccelerationStructureRef blas = AccelerationStructure::Create(&asCI);

	asbiGBI.dstAccelerationStructure = blas;
	asbiGBI.scratchData.deviceAddress = GetBufferDeviceAddress(context->GetDevice(), scratchBuffer_BLAS);
	blas_asbi = AccelerationStructureBuildInfo::Create(&asbiGBI);

	AccelerationStructureBuildInfo::BuildRangeInfo blas_bri;
	blas_bri.primitiveCount = asbiGBI.maxPrimitiveCounts;
	blas_bri.primitiveOffset = 0;
	blas_bri.firstVertex = 0;
	blas_bri.transformOffset = 0;

	//TLAS
	InstanceData id;
	id.transform = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };
	id.instanceCustomIndex = 0;
	id.mask = 0xFF;
	id.instanceShaderBindingTableRecordOffset = 0;
	id.flags = (uint32_t)InstanceDataFlagBit::NONE_BIT;
	id.accelerationStructureReference = GetAccelerationStructureDeviceAddress(context->GetDevice(), blas);

	Buffer::CreateInfo idBufferCI;
	idBufferCI.debugName = "TLASInstanceData";
	idBufferCI.device = context->GetDevice();
	idBufferCI.usage = Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	idBufferCI.size = sizeof(InstanceData);
	idBufferCI.data = &id;
	idBufferCI.allocator = cpu_alloc_0;
	BufferRef idBuffer_TLAS = Buffer::Create(&idBufferCI);

	asbiGBI.device = context->GetDevice();
	asbiGBI.type = AccelerationStructureBuildInfo::BuildGeometryInfo::Type::TOP_LEVEL;
	asbiGBI.flags = AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::PREFER_FAST_TRACE_BIT;
	asbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::BUILD;
	asbiGBI.srcAccelerationStructure = nullptr;
	asbiGBI.dstAccelerationStructure = nullptr;
	asbiGBI.geometries.clear();
	asbiGBI.geometries.push_back({});
	asbiGBI.geometries[0].type = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::Type::INSTANCES;
	asbiGBI.geometries[0].instances = { false, GetBufferDeviceAddress(context->GetDevice(), idBuffer_TLAS) };
	asbiGBI.geometries[0].flags = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::FlagBit::OPAQUE_BIT;
	asbiGBI.scratchData = DeviceOrHostAddressNull;
	asbiGBI.buildType = AccelerationStructureBuildInfo::BuildGeometryInfo::BuildType::DEVICE;
	asbiGBI.maxPrimitiveCounts = 1;
	AccelerationStructureBuildInfoRef tlas_asbi = AccelerationStructureBuildInfo::Create(&asbiGBI);

	asBufferCI.debugName = "TLASBuffer";
	asBufferCI.device = context->GetDevice();
	asBufferCI.usage = Buffer::UsageBit::ACCELERATION_STRUCTURE_STORAGE_BIT;
	asBufferCI.size = tlas_asbi->GetBuildSizesInfo().accelerationStructureSize;
	asBufferCI.data = nullptr;
	asBufferCI.allocator = gpu_alloc_0;
	BufferRef asBuffer_TLAS = Buffer::Create(&asBufferCI);

	scratchBufferCI.debugName = "TLASScratchBuffer";
	scratchBufferCI.device = context->GetDevice();
	scratchBufferCI.usage = Buffer::UsageBit::STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	scratchBufferCI.size = tlas_asbi->GetBuildSizesInfo().buildScratchSize;
	scratchBufferCI.data = nullptr;
	scratchBufferCI.allocator = gpu_alloc_0;
	BufferRef scratchBuffer_TLAS = Buffer::Create(&scratchBufferCI);

	asCI.debugName = "TLAS";
	asCI.device = context->GetDevice();
	asCI.flags = AccelerationStructure::FlagBit::NONE_BIT;
	asCI.buffer = asBuffer_TLAS;
	asCI.offset = 0;
	asCI.size = asBufferCI.size;
	asCI.type = AccelerationStructure::Type::TOP_LEVEL;
	asCI.deviceAddress = DeviceAddressNull;
	AccelerationStructureRef tlas = AccelerationStructure::Create(&asCI);

	asbiGBI.dstAccelerationStructure = tlas;
	asbiGBI.scratchData.deviceAddress = GetBufferDeviceAddress(context->GetDevice(), scratchBuffer_TLAS);
	tlas_asbi = AccelerationStructureBuildInfo::Create(&asbiGBI);

	AccelerationStructureBuildInfo::BuildRangeInfo tlas_bri;
	tlas_bri.primitiveCount = asbiGBI.maxPrimitiveCounts;
	tlas_bri.primitiveOffset = 0;
	tlas_bri.firstVertex = 0;
	tlas_bri.transformOffset = 0;

	//Transfer CmdBuffer Record and Submit
	Fence::CreateInfo transferFenceCI = { "TransferFence", context->GetDevice(), false, UINT64_MAX };
	FenceRef transferFence = Fence::Create(&transferFenceCI);
	{
		cmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
		bCI.dstAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.image = RT_RWImage;
		bCI.oldLayout = Image::Layout::UNKNOWN;
		bCI.newLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
		bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		BarrierRef b = Barrier::Create(&bCI);
		cmdBuffer->PipelineBarrier(2, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::RAY_TRACING_SHADER_BIT, DependencyBit::NONE_BIT, { b });

		cmdBuffer->BuildAccelerationStructure(2, { blas_asbi, tlas_asbi }, { { blas_bri }, { tlas_bri } });

		cmdBuffer->End(2);
	}
	CommandBuffer::SubmitInfo copySI = { { 2 }, {}, {}, {}, {}, {} };
	cmdBuffer->Submit({ copySI }, transferFence);
	transferFence->Wait();

	//Ray tracing descriptor sets and pipeline
	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "RayTracing: Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::UNIFORM_BUFFER, 1}, {DescriptorType::STORAGE_IMAGE, 1}, {DescriptorType::ACCELERATION_STRUCTURE, 1} };
	descriptorPoolCI.maxSets = 1;
	DescriptorPoolRef descriptorPoolRT = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "RayTracing: DescSetLayout1";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{1, DescriptorType::STORAGE_IMAGE, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{2, DescriptorType::ACCELERATION_STRUCTURE, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT }
	};
	DescriptorSetLayoutRef setLayout1RT = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetRTCI;
	descriptorSetRTCI.debugName = "RayTracing: DescSet1";
	descriptorSetRTCI.descriptorPool = descriptorPoolRT;
	descriptorSetRTCI.descriptorSetLayouts = { setLayout1RT };
	DescriptorSetRef descriptorSetRT_Global = DescriptorSet::Create(&descriptorSetRTCI);
	descriptorSetRT_Global->AddBuffer(0, 0, { { ubViewSceneConstants } });
	descriptorSetRT_Global->AddImage(0, 1, { {nullptr, RT_RWImageView, Image::Layout::GENERAL} });
	descriptorSetRT_Global->AddAccelerationStructure(0, 2, { tlas });
	descriptorSetRT_Global->Update();

	Pipeline::CreateInfo raytracingPipelineCI;
	raytracingPipelineCI.debugName = "Ray Tracing Pipeline";
	raytracingPipelineCI.device = context->GetDevice();
	raytracingPipelineCI.type = PipelineType::RAY_TRACING;
	raytracingPipelineCI.shaders = { raytracingShader };
	raytracingPipelineCI.dynamicStates = {};
	raytracingPipelineCI.shaderGroupInfos = {
		{ ShaderGroupType::GENERAL, 0, Pipeline::ShaderUnused, Pipeline::ShaderUnused, Pipeline::ShaderUnused },
		{ ShaderGroupType::TRIANGLES_HIT_GROUP, Pipeline::ShaderUnused, 1, 2, Pipeline::ShaderUnused, },
		{ ShaderGroupType::GENERAL, 3, Pipeline::ShaderUnused, Pipeline::ShaderUnused, Pipeline::ShaderUnused },
	};
	raytracingPipelineCI.rayTracingInfo = { 1, 16, 8, cpu_alloc_0 };
	raytracingPipelineCI.layout = { {setLayout1RT }, {} };
	PipelineRef raytracingPipeline = Pipeline::Create(&raytracingPipelineCI);
	auto handles = raytracingPipeline->GetShaderGroupHandles();

	//Shader Binding Table
	ShaderBindingTable::CreateInfo sbtCI;
	sbtCI.debugName = "Ray Tracing Pipeline";
	sbtCI.device = context->GetDevice();
	sbtCI.shaderRecords = {
		{ handles[0].first, handles[0].second, {} },
		{ handles[1].first, handles[1].second, {} },
		{ handles[2].first, handles[2].second, {} }
	};
	sbtCI.allocator = cpu_alloc_0;
	ShaderBindingTableRef sbt = ShaderBindingTable::Create(&sbtCI);

	//Render Synchronisation
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
	MSG msg = { 0 };
	//Main Render Loop
	while (!g_WindowQuit)
	{
		WindowUpdate();

		if (shaderRecompile)
		{
			context->DeviceWaitIdle();

			raytracingShader->Recompile();
			raytracingPipelineCI.shaders = { raytracingShader };
			raytracingPipeline = Pipeline::Create(&raytracingPipelineCI);
			auto handles = raytracingPipeline->GetShaderGroupHandles();

			sbtCI.shaderRecords = {
				{ handles[0].first, handles[0].second, {} },
				{ handles[1].first, handles[1].second, {} },
				{ handles[2].first, handles[2].second, {} }
			};
			sbt = ShaderBindingTable::Create(&sbtCI);
			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			shaderRecompile = false;
			frameIndex = 0;
		}
		if (swapchain->m_Resized || windowResize)
		{
			swapchain->Resize(width, height);

			RT_RWImageCI.width = width;
			RT_RWImageCI.height = height;
			RT_RWImage = Image::Create(&RT_RWImageCI);
			RT_RWImageViewCI.image = RT_RWImage;
			RT_RWImageView = ImageView::Create(&RT_RWImageViewCI);

			descriptorSetRT_Global = nullptr;
			descriptorSetRT_Global = DescriptorSet::Create(&descriptorSetRTCI);
			descriptorSetRT_Global->AddBuffer(0, 0, { { ubViewSceneConstants } });
			descriptorSetRT_Global->AddImage(0, 1, { {nullptr, RT_RWImageView, Image::Layout::GENERAL} });
			descriptorSetRT_Global->AddAccelerationStructure(0, 2, { tlas });
			descriptorSetRT_Global->Update();

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquire = Semaphore::Create(&acquireSemaphoreCI);
			submit = Semaphore::Create(&submitSemaphoreCI);

			swapchain->m_Resized = false;
			windowResize = false;

			//Transition any resource into the correct states.
			{
				cmdBuffer->Reset(2, false);
				cmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

				Barrier::CreateInfo bCI;
				bCI.type = Barrier::Type::IMAGE;
				bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
				bCI.dstAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
				bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
				bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
				bCI.image = RT_RWImage;
				bCI.oldLayout = Image::Layout::UNKNOWN;
				bCI.newLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
				bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
				BarrierRef b = Barrier::Create(&bCI);
				cmdBuffer->PipelineBarrier(2, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::RAY_TRACING_SHADER_BIT, DependencyBit::NONE_BIT, { b });

				cmdBuffer->End(2);
			}
			CommandBuffer::SubmitInfo si = { { 2 }, {}, {}, {}, {}, {}, };
			cmdBuffer->Submit({ si }, nullptr);
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
			cmdBuffer->BindPipeline(frameIndex, raytracingPipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSetRT_Global }, 0, raytracingPipeline);
			cmdBuffer->TraceRays(frameIndex, &sbt->GetStridedDeviceAddressRegion(ShaderGroupHandleType::RAYGEN), &sbt->GetStridedDeviceAddressRegion(ShaderGroupHandleType::MISS), &sbt->GetStridedDeviceAddressRegion(ShaderGroupHandleType::HIT_GROUP), nullptr, width, height, 1);

			Barrier::CreateInfo bCI;
			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::SHADER_READ_BIT;
			bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.image = RT_RWImage;
			bCI.oldLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
			bCI.newLayout = Image::Layout::TRANSFER_SRC_OPTIMAL;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			BarrierRef b1 = Barrier::Create(&bCI);

			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT;
			bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.image = swapchain->m_SwapchainImages[frameIndex];
			bCI.oldLayout = Image::Layout::UNKNOWN;
			bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			BarrierRef b2 = Barrier::Create(&bCI);
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::RAY_TRACING_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b1 });
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b2 });

			cmdBuffer->CopyImage(frameIndex, RT_RWImage, Image::Layout::TRANSFER_SRC_OPTIMAL,
				swapchain->m_SwapchainImages[frameIndex], Image::Layout::TRANSFER_DST_OPTIMAL,
				{ { {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0, 0, 0}, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0, 0, 0},
				{RT_RWImage->GetCreateInfo().width, RT_RWImage->GetCreateInfo().height, 1} } });

			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
			bCI.dstAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.image = RT_RWImage;
			bCI.oldLayout = Image::Layout::TRANSFER_SRC_OPTIMAL;
			bCI.newLayout = GraphicsAPI::IsD3D12() ? Image::Layout::D3D12_UNORDERED_ACCESS : Image::Layout::GENERAL;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			b1 = Barrier::Create(&bCI);

			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
			bCI.dstAccess = Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.image = swapchain->m_SwapchainImages[frameIndex];
			bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.newLayout = Image::Layout::PRESENT_SRC;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			b2 = Barrier::Create(&bCI);
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::RAY_TRACING_SHADER_BIT, DependencyBit::NONE_BIT, { b1 });
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, DependencyBit::NONE_BIT, { b2 });
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

			frameCount++;
		}
	}
	context->DeviceWaitIdle();
}
#endif