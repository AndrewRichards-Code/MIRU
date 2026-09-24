#include "miru_core.h"
#include "common.h"
#include "maths.h"

#include "stb/stb_image.h"

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

// Test Hardware accelerated raytracing, acceleration structures and raytracing pipeline.
// One Trace call to the swapchain. Barycentric colours on the geometry via hit shaders and background colour via miss shader.
void Raytracing(uint32_t maxFrames)
{
	MIRU_CPU_PROFILE_BEGIN_SESSION("miru_profile_result.txt");

	Instance::CreateInfo instanceCI;
	instanceCI.applicationName = "MIRU_TEST_Raytracing";
	instanceCI.debugValidationLayers = true;
	instanceCI.pNext = nullptr;
	InstanceRef instance = Instance::Create(&instanceCI);

	PhysicalDeviceRefs physicalDevices = Instance::GetPhysicalDevices(instance);
	PhysicalDeviceRef physicalDevice = physicalDevices[0]; //Pick one?

	Device::CreateInfo deviceCI;
	deviceCI.physicalDevice = physicalDevice;
	deviceCI.debugValidationLayers = true;
	deviceCI.extensions = Device::ExtensionsBit::RAY_TRACING | Device::ExtensionsBit::DESCRIPTOR_INDEXING;
	deviceCI.debugName = "GPU Device";
	DeviceRef device = Device::Create(&deviceCI);

	//Creates the windows
	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = instanceCI.applicationName.c_str();
	RegisterClass(&wc);

	window = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, 100, 100, width, height, 0, 0, 0, 0);
	ShowWindow(window, SW_SHOW);

	Swapchain::CreateInfo swapchainCI;
	swapchainCI.debugName = "Swapchain";
	swapchainCI.device = device;
	swapchainCI.pWindow = window;
	swapchainCI.width = width;
	swapchainCI.height = height;
	swapchainCI.swapchainCount = 2;
	swapchainCI.vSync = true;
	SwapchainRef swapchain = Swapchain::Create(&swapchainCI);
	const Extent2D& swapchainDimensions = swapchain->GetSwapchainDimensions();
	width = swapchainDimensions.width;
	height = swapchainDimensions.height;

	//Ray Tracing library
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "RayTracing: Library Shader Module";
	shaderCI.device = device;
	shaderCI.stageAndEntryPoints = {
		{ Shader::StageBit::RAYGEN_BIT, "ray_generation_main"},
		{ Shader::StageBit::ANY_HIT_BIT, "any_hit_main"},
		{ Shader::StageBit::CLOSEST_HIT_BIT, "closest_hit_main"},
		{ Shader::StageBit::MISS_BIT, "miss_main"},
	};
	shaderCI.binaryFilepath = "../shaderbin/raytracing_lib_6_3.spv";
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments = base::Shader::LoadCompileArgumentsFromFile("../shaderbin/raytracing_hlsl.json", { { "$SOLUTION_DIR", SOLUTION_DIR }, { "$BUILD_DIR", BUILD_DIR } })[0];
	ShaderRef raytracingShader = Shader::Create(&shaderCI);

	//CmdPool and CmdBuffer
	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CmdPool";
	cmdPoolCI.device = device;
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
	allocCI.device = device;
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

	int img_width;
	int img_height;
	int bpp;
	std::string logoFilepath = std::string(SOLUTION_DIR) + std::string("/Branding/logo.png");
	uint8_t* imageData = stbi_load(logoFilepath.c_str(), &img_width, &img_height, &bpp, 4);

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices Buffer";
	verticesBufferCI.device = device;
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	verticesBufferCI.size = sizeof(vertices);
	verticesBufferCI.data = vertices;
	verticesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_vb = Buffer::Create(&verticesBufferCI);
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT | Buffer::UsageBit::STORAGE_BIT;
	verticesBufferCI.data = nullptr;
	verticesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_vb = Buffer::Create(&verticesBufferCI);

	Buffer::CreateInfo indicesBufferCI;
	indicesBufferCI.debugName = "Indices Buffer";
	indicesBufferCI.device = device;
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.allocator = cpu_alloc_0;
	BufferRef c_ib = Buffer::Create(&indicesBufferCI);
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT | Buffer::UsageBit::STORAGE_BIT;
	indicesBufferCI.data = nullptr;
	indicesBufferCI.allocator = gpu_alloc_0;
	BufferRef g_ib = Buffer::Create(&indicesBufferCI);

	Buffer::CreateInfo imageBufferCI;
	imageBufferCI.debugName = "MIRU logo upload buffer";
	imageBufferCI.device = device;
	imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	imageBufferCI.size = img_width * img_height * 4;
	imageBufferCI.data = imageData;
	imageBufferCI.allocator = cpu_alloc_0;
	BufferRef c_imageBuffer = Buffer::Create(&imageBufferCI);
	stbi_image_free(imageData);

	Image::CreateInfo imageCI;
	imageCI.debugName = "MIRU logo Image";
	imageCI.device = device;
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
	imageCI.allocator = gpu_alloc_0;
	imageCI.externalImage = nullptr;
	ImageRef image = Image::Create(&imageCI);

	//Uniform buffers
	Mat4 proj = Mat4::Perspective(3.14159 / 2.0, float(width) / float(height), 0.1f, 100.0f);
	Mat4 view = Mat4::Identity();
	Mat4 modl = Mat4::Translation({ 0.0f, 0.0f, -1.5f });

	float ubData[2 * sizeof(Mat4)];
	memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
	memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

	Buffer::CreateInfo ubCI;
	ubCI.debugName = "Model UB";
	ubCI.device = device;
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	ubCI.size = sizeof(Mat4);
	ubCI.data = &modl.a;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ub1 = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubViewMdlCI;
	ubViewMdlCI.debugName = "Model UBView";
	ubViewMdlCI.device = device;
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
	ubCI.device = device;
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = sizeof(SceneConstants);
	ubCI.data = &ubSceneConstantsData;
	ubCI.allocator = cpu_alloc_0;
	BufferRef ubSceneConstants = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubSceneConstantsCI;
	ubSceneConstantsCI.debugName = "RTShaderConstants UBView";
	ubSceneConstantsCI.device = device;
	ubSceneConstantsCI.type = BufferView::Type::UNIFORM;
	ubSceneConstantsCI.buffer = ubSceneConstants;
	ubSceneConstantsCI.offset = 0;
	ubSceneConstantsCI.size = sizeof(SceneConstants);
	ubSceneConstantsCI.stride = 0;
	BufferViewRef ubViewSceneConstants = BufferView::Create(&ubSceneConstantsCI);

	BufferView::CreateInfo vbViewCI;
	vbViewCI.debugName = "VerticesBufferView";
	vbViewCI.device = device;
	vbViewCI.type = BufferView::Type::STORAGE;
	vbViewCI.buffer = g_vb;
	vbViewCI.offset = 0;
	vbViewCI.size = sizeof(vertices);
	vbViewCI.stride = 4 * sizeof(float);
	BufferViewRef vbv = BufferView::Create(&vbViewCI);

	BufferView::CreateInfo ibViewCI;
	ibViewCI.debugName = "IndicesBufferView";
	ibViewCI.device = device;
	ibViewCI.type = BufferView::Type::STORAGE;
	ibViewCI.buffer = g_ib;
	ibViewCI.offset = 0;
	ibViewCI.size = sizeof(indices);
	ibViewCI.stride = sizeof(uint32_t);
	BufferViewRef ibv = BufferView::Create(&ibViewCI);

	//RW Image
	Image::CreateInfo RT_RWImageCI;
	RT_RWImageCI.debugName = "RT_RWImage";
	RT_RWImageCI.device = device;
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
	RT_RWImageViewCI.device = device;
	RT_RWImageViewCI.image = RT_RWImage;
	RT_RWImageViewCI.viewType = Image::Type::TYPE_2D;
	RT_RWImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	ImageViewRef RT_RWImageView = ImageView::Create(&RT_RWImageViewCI);

	//Acceleration structure building
	//BLAS
	AccelerationStructureBuildInfo::BuildGeometryInfo blasbiGBI;
	blasbiGBI.device = device;
	blasbiGBI.type = AccelerationStructureBuildInfo::BuildGeometryInfo::Type::BOTTOM_LEVEL;
	blasbiGBI.flags = AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::PREFER_FAST_TRACE_BIT | AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::ALLOW_UPDATE_BIT;
	blasbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::BUILD;
	blasbiGBI.srcAccelerationStructure = nullptr;
	blasbiGBI.dstAccelerationStructure = nullptr;
	blasbiGBI.geometries.clear();
	blasbiGBI.geometries.push_back({});
	blasbiGBI.geometries[0].type = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::Type::TRIANGLES;
	blasbiGBI.geometries[0].triangles = {
				VertexType::FLOAT3,
				GetBufferDeviceAddress(device, c_vb),
				static_cast<uint64_t>(4 * sizeof(float)),
				std::size(vertices) / 4,
				IndexType::UINT32,
				GetBufferDeviceAddress(device, c_ib),
				std::size(indices),
				GetBufferDeviceAddress(device, ub1)
	};
	blasbiGBI.geometries[0].flags = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::FlagBit::OPAQUE_BIT;
	blasbiGBI.scratchData = DeviceOrHostAddressNull;
	blasbiGBI.buildType = AccelerationStructureBuildInfo::BuildGeometryInfo::BuildType::DEVICE;
	blasbiGBI.maxPrimitiveCounts.clear();
	blasbiGBI.maxPrimitiveCounts.push_back({});
	blasbiGBI.maxPrimitiveCounts[0] = std::size(indices) / 3;
	AccelerationStructureBuildInfoRef blas_asbi = AccelerationStructureBuildInfo::Create(&blasbiGBI);

	Buffer::CreateInfo blasBufferCI;
	blasBufferCI.debugName = "BLASBuffer";
	blasBufferCI.device = device;
	blasBufferCI.usage = Buffer::UsageBit::ACCELERATION_STRUCTURE_STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	blasBufferCI.size = blas_asbi->GetBuildSizesInfo().accelerationStructureSize;
	blasBufferCI.data = nullptr;
	blasBufferCI.allocator = gpu_alloc_0;
	BufferRef blasBuffer_BLAS = Buffer::Create(&blasBufferCI);

	Buffer::CreateInfo blasScratchBufferCI;
	blasScratchBufferCI.debugName = "BLASScratchBuffer";
	blasScratchBufferCI.device = device;
	blasScratchBufferCI.usage = Buffer::UsageBit::STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	blasScratchBufferCI.size = blas_asbi->GetBuildSizesInfo().buildScratchSize;
	blasScratchBufferCI.data = nullptr;
	blasScratchBufferCI.allocator = gpu_alloc_0;
	BufferRef scratchBuffer_BLAS = Buffer::Create(&blasScratchBufferCI);

	AccelerationStructure::CreateInfo blasCI;
	blasCI.debugName = "BLAS";
	blasCI.device = device;
	blasCI.flags = AccelerationStructure::FlagBit::NONE_BIT;
	blasCI.buffer = blasBuffer_BLAS;
	blasCI.offset = 0;
	blasCI.size = blasBufferCI.size;
	blasCI.type = AccelerationStructure::Type::BOTTOM_LEVEL;
	blasCI.deviceAddress = DeviceAddressNull;
	AccelerationStructureRef blas = AccelerationStructure::Create(&blasCI);

	blasbiGBI.dstAccelerationStructure = blas;
	blasbiGBI.scratchData.deviceAddress = GetBufferDeviceAddress(device, scratchBuffer_BLAS);
	blas_asbi = AccelerationStructureBuildInfo::Create(&blasbiGBI);

	AccelerationStructureBuildInfo::BuildRangeInfo blas_bri;
	blas_bri.primitiveCount = blasbiGBI.maxPrimitiveCounts[0];
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
	id.accelerationStructureReference = GetAccelerationStructureDeviceAddress(device, blas);

	Buffer::CreateInfo idBufferCI;
	idBufferCI.debugName = "TLASInstanceData";
	idBufferCI.device = device;
	idBufferCI.usage = Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	idBufferCI.size = sizeof(InstanceData);
	idBufferCI.data = &id;
	idBufferCI.allocator = cpu_alloc_0;
	BufferRef idBuffer_TLAS = Buffer::Create(&idBufferCI);

	AccelerationStructureBuildInfo::BuildGeometryInfo tlasbiGBI;
	tlasbiGBI.device = device;
	tlasbiGBI.type = AccelerationStructureBuildInfo::BuildGeometryInfo::Type::TOP_LEVEL;
	tlasbiGBI.flags = AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::PREFER_FAST_TRACE_BIT | AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::ALLOW_UPDATE_BIT;
	tlasbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::BUILD;
	tlasbiGBI.srcAccelerationStructure = nullptr;
	tlasbiGBI.dstAccelerationStructure = nullptr;
	tlasbiGBI.geometries.clear();
	tlasbiGBI.geometries.push_back({});
	tlasbiGBI.geometries[0].type = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::Type::INSTANCES;
	tlasbiGBI.geometries[0].instances = { false, GetBufferDeviceAddress(device, idBuffer_TLAS) };
	tlasbiGBI.geometries[0].flags = AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::FlagBit::OPAQUE_BIT;
	tlasbiGBI.scratchData = DeviceOrHostAddressNull;
	tlasbiGBI.buildType = AccelerationStructureBuildInfo::BuildGeometryInfo::BuildType::DEVICE;
	tlasbiGBI.maxPrimitiveCounts.clear();
	tlasbiGBI.maxPrimitiveCounts.push_back({});
	tlasbiGBI.maxPrimitiveCounts[0] = 1;
	AccelerationStructureBuildInfoRef tlas_asbi = AccelerationStructureBuildInfo::Create(&tlasbiGBI);

	Buffer::CreateInfo tlasBufferCI;
	tlasBufferCI.debugName = "TLASBuffer";
	tlasBufferCI.device = device;
	tlasBufferCI.usage = Buffer::UsageBit::ACCELERATION_STRUCTURE_STORAGE_BIT;
	tlasBufferCI.size = tlas_asbi->GetBuildSizesInfo().accelerationStructureSize;
	tlasBufferCI.data = nullptr;
	tlasBufferCI.allocator = gpu_alloc_0;
	BufferRef asBuffer_TLAS = Buffer::Create(&tlasBufferCI);

	Buffer::CreateInfo tlasScratchBufferCI;
	tlasScratchBufferCI.debugName = "TLASScratchBuffer";
	tlasScratchBufferCI.device = device;
	tlasScratchBufferCI.usage = Buffer::UsageBit::STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	tlasScratchBufferCI.size = tlas_asbi->GetBuildSizesInfo().buildScratchSize;
	tlasScratchBufferCI.data = nullptr;
	tlasScratchBufferCI.allocator = gpu_alloc_0;
	BufferRef scratchBuffer_TLAS = Buffer::Create(&tlasScratchBufferCI);

	AccelerationStructure::CreateInfo tlasCI;
	tlasCI.debugName = "TLAS";
	tlasCI.device = device;
	tlasCI.flags = AccelerationStructure::FlagBit::NONE_BIT;
	tlasCI.buffer = asBuffer_TLAS;
	tlasCI.offset = 0;
	tlasCI.size = tlasBufferCI.size;
	tlasCI.type = AccelerationStructure::Type::TOP_LEVEL;
	tlasCI.deviceAddress = DeviceAddressNull;
	AccelerationStructureRef tlas = AccelerationStructure::Create(&tlasCI);

	tlasbiGBI.dstAccelerationStructure = tlas;
	tlasbiGBI.scratchData.deviceAddress = GetBufferDeviceAddress(device, scratchBuffer_TLAS);
	tlas_asbi = AccelerationStructureBuildInfo::Create(&tlasbiGBI);

	AccelerationStructureBuildInfo::BuildRangeInfo tlas_bri;
	tlas_bri.primitiveCount = tlasbiGBI.maxPrimitiveCounts[0];
	tlas_bri.primitiveOffset = 0;
	tlas_bri.firstVertex = 0;
	tlas_bri.transformOffset = 0;

	//Transfer CmdBuffer Record and Submit
	Fence::CreateInfo transferFenceCI = { "TransferFence", device, false, UINT64_MAX };
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

		cmdBuffer->BuildAccelerationStructures(2, { blas_asbi, tlas_asbi }, { { blas_bri }, { tlas_bri } });

		cmdBuffer->CopyBuffer(2, c_vb, g_vb, { { 0, 0, sizeof(vertices) } });
		cmdBuffer->CopyBuffer(2, c_ib, g_ib, { { 0, 0, sizeof(indices) } });

		if (GraphicsAPI::IsVulkan())
		{
			Barrier::CreateInfo bCI;
			bCI.type = Barrier::Type::IMAGE;
			bCI.srcAccess = Barrier::AccessBit::NONE_BIT;
			bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
			bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
			bCI.image = image;
			bCI.oldLayout = Image::Layout::UNKNOWN;
			bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			BarrierRef b = Barrier::Create(&bCI);
			cmdBuffer->PipelineBarrier(2, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b });
		}
		cmdBuffer->CopyBufferToImage(2, c_imageBuffer, image, Image::Layout::TRANSFER_DST_OPTIMAL, {
			{0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0,0,0}, {imageCI.width, imageCI.height, imageCI.depth}}
			});

		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		bCI.srcQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.dstQueueFamilyIndex = Barrier::QueueFamilyIgnored;
		bCI.image = image;
		bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		b = Barrier::Create(&bCI);
		cmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::RAY_TRACING_SHADER_BIT, DependencyBit::NONE_BIT, { b });

		cmdBuffer->End(2);
	}
	CommandBuffer::SubmitInfo copySI = { { 2 }, {}, {}, {}, {}, {} };
	cmdBuffer->Submit({ copySI }, transferFence);
	transferFence->Wait();

	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "MIRU logo ImageView";
	imageViewCI.device = device;
	imageViewCI.image = image;
	imageViewCI.viewType = Image::Type::TYPE_2D;
	imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	ImageViewRef imageView = ImageView::Create(&imageViewCI);

	Sampler::CreateInfo samplerCI;
	samplerCI.debugName = "Default Sampler";
	samplerCI.device = device;
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

	//Ray tracing descriptor sets and pipeline
	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "RayTracing: Descriptor Pool";
	descriptorPoolCI.device = device;
	descriptorPoolCI.poolSizes = { {DescriptorType::UNIFORM_BUFFER, 1}, {DescriptorType::STORAGE_IMAGE, 1}, {DescriptorType::ACCELERATION_STRUCTURE, 1}, {DescriptorType::STORAGE_BUFFER, 2}, {DescriptorType::SAMPLER, 1}, {DescriptorType::SAMPLED_IMAGE, DescriptorUnboundedArrayCount} };
	descriptorPoolCI.maxSets = 2;
	DescriptorPoolRef descriptorPoolRT = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "RayTracing: DescSetLayout1";
	setLayoutCI.device = device;
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{1, DescriptorType::STORAGE_IMAGE, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{2, DescriptorType::ACCELERATION_STRUCTURE, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT }
	};
	DescriptorSetLayoutRef setLayout1RT = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "RayTracing: DescSetLayout2";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::SAMPLER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{1, GraphicsAPI::IsD3D12() ? DescriptorType::D3D12_STRUCTURED_BUFFER : DescriptorType::STORAGE_BUFFER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{2, GraphicsAPI::IsD3D12() ? DescriptorType::D3D12_STRUCTURED_BUFFER : DescriptorType::STORAGE_BUFFER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{3, DescriptorType::SAMPLED_IMAGE, DescriptorUnboundedArrayCount, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT, DescriptorSetLayout::Binding::FlagBit::PARTIALLY_BOUND_BIT | DescriptorSetLayout::Binding::FlagBit::VARIABLE_DESCRIPTOR_COUNT_BIT },
	};
	DescriptorSetLayoutRef setLayout2RT = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetRTCI;
	descriptorSetRTCI.debugName = "RayTracing: DescSet1";
	descriptorSetRTCI.descriptorPool = descriptorPoolRT;
	descriptorSetRTCI.descriptorSetLayouts = { setLayout1RT, setLayout2RT };
	descriptorSetRTCI.descriptorCounts = { 0, 16 };
	DescriptorSetRef descriptorSetRT = DescriptorSet::Create(&descriptorSetRTCI);
	descriptorSetRT->AddBuffer(0, 0, { { ubViewSceneConstants } });
	descriptorSetRT->AddImage(0, 1, { {nullptr, RT_RWImageView, Image::Layout::GENERAL} });
	descriptorSetRT->AddAccelerationStructure(0, 2, { tlas });
	descriptorSetRT->AddImage(1, 0, { {sampler, nullptr, Image::Layout::UNKNOWN} });
	descriptorSetRT->AddBuffer(1, 1, { { vbv } });
	descriptorSetRT->AddBuffer(1, 2, { { ibv } });
	descriptorSetRT->AddImage(1, 3, { {nullptr, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL} });
	descriptorSetRT->Update();

	Pipeline::CreateInfo raytracingPipelineCI;
	raytracingPipelineCI.debugName = "Ray Tracing Pipeline";
	raytracingPipelineCI.device = device;
	raytracingPipelineCI.type = PipelineType::RAY_TRACING;
	raytracingPipelineCI.shaders = { raytracingShader };
	raytracingPipelineCI.dynamicStates = {};
	raytracingPipelineCI.shaderGroupInfos = {
		{ ShaderGroupType::GENERAL, 0, Pipeline::ShaderUnused, Pipeline::ShaderUnused, Pipeline::ShaderUnused },
		{ ShaderGroupType::TRIANGLES_HIT_GROUP, Pipeline::ShaderUnused, 1, 2, Pipeline::ShaderUnused, },
		{ ShaderGroupType::GENERAL, 3, Pipeline::ShaderUnused, Pipeline::ShaderUnused, Pipeline::ShaderUnused },
	};
	raytracingPipelineCI.rayTracingInfo = { 1, 16, 8, cpu_alloc_0 };
	raytracingPipelineCI.layout = { { setLayout1RT, setLayout2RT }, {} };
	PipelineRef raytracingPipeline = Pipeline::Create(&raytracingPipelineCI);
	auto handles = raytracingPipeline->GetShaderGroupHandles();

	//Shader Binding Table
	ShaderBindingTable::CreateInfo sbtCI;
	sbtCI.debugName = "Ray Tracing Pipeline";
	sbtCI.device = device;
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
	fenceCI.device = device;
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<FenceRef> draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
	Semaphore::CreateInfo acquireSemaphoreCI = { "AcquireSemaphore", device };
	Semaphore::CreateInfo submitSemaphoreCI = { "SubmitSemaphore", device };
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
	// https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html

	//Main Render Loop
	while (!g_WindowQuit && frameCount < maxFrames)
	{
		WindowUpdate();

		if (shaderRecompile)
		{
			device->DeviceWaitIdle();

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

			shaderRecompile = false;
		}
		if (swapchain->m_Resized || windowResize)
		{
			swapchain->Resize(width, height);

			RT_RWImageCI.width = width;
			RT_RWImageCI.height = height;
			RT_RWImage = Image::Create(&RT_RWImageCI);
			RT_RWImageViewCI.image = RT_RWImage;
			RT_RWImageView = ImageView::Create(&RT_RWImageViewCI);

			descriptorSetRT = nullptr;
			descriptorSetRT = DescriptorSet::Create(&descriptorSetRTCI);
			descriptorSetRT->AddBuffer(0, 0, { { ubViewSceneConstants } });
			descriptorSetRT->AddImage(0, 1, { {nullptr, RT_RWImageView, Image::Layout::GENERAL} });
			descriptorSetRT->AddAccelerationStructure(0, 2, { tlas });
			descriptorSetRT->AddImage(1, 0, { {sampler, nullptr, Image::Layout::UNKNOWN} });
			descriptorSetRT->AddBuffer(1, 1, { { vbv } });
			descriptorSetRT->AddBuffer(1, 2, { { ibv } });
			descriptorSetRT->AddImage(1, 3, { {nullptr, imageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL} });
			descriptorSetRT->Update();

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquires = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
			submits = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };

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
			Fence::CreateInfo updateFenceCI = { "UpdateFence", device, false, UINT64_MAX };
			FenceRef updateFence = Fence::Create(&updateFenceCI);
			CommandBuffer::SubmitInfo si = { { 2 }, {}, {}, {}, {}, {}, };
			cmdBuffer->Submit({ si }, updateFence);
			updateFence->Wait();
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
			cmdBuffer->BindPipeline(frameIndex, raytracingPipeline);
			cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSetRT }, 0, raytracingPipeline);
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
			bCI.image = swapchain->m_SwapchainImages[swapchainImageIndex];
			bCI.oldLayout = Image::Layout::UNKNOWN;
			bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			BarrierRef b2 = Barrier::Create(&bCI);
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::RAY_TRACING_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b1 });
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b2 });

			cmdBuffer->CopyImage(frameIndex, RT_RWImage, Image::Layout::TRANSFER_SRC_OPTIMAL,
				swapchain->m_SwapchainImages[swapchainImageIndex], Image::Layout::TRANSFER_DST_OPTIMAL,
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
			bCI.image = swapchain->m_SwapchainImages[swapchainImageIndex];
			bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
			bCI.newLayout = Image::Layout::PRESENT_SRC;
			bCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
			b2 = Barrier::Create(&bCI);
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::RAY_TRACING_SHADER_BIT, DependencyBit::NONE_BIT, { b1 });
			cmdBuffer->PipelineBarrier(frameIndex, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, DependencyBit::NONE_BIT, { b2 });
			cmdBuffer->End(frameIndex);
			
			CommandBuffer::SubmitInfo mainSI = { { frameIndex }, { acquires[frameIndex] }, {}, { base::PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT }, { submits[swapchainImageIndex] }, {} };
			cmdBuffer->Submit({ mainSI }, draws[frameIndex]);

			swapchain->Present(cmdPool, submits[swapchainImageIndex], swapchainImageIndex);

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

			//cpu_alloc_0->SubmitData(ub1->GetAllocation(), 0, 2 * sizeof(Mat4), ubData);
			cpu_alloc_0->SubmitData(ub1->GetAllocation(), 0, sizeof(Mat4), (void*)&modl.a);

			//Update BLAS and TLAS
			{
				blasbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::UPDATE;
				blasbiGBI.srcAccelerationStructure = blas;
				blasbiGBI.dstAccelerationStructure = blas;
				blas_asbi = AccelerationStructureBuildInfo::Create(&blasbiGBI);

				blas_bri.primitiveCount = blasbiGBI.maxPrimitiveCounts[0];
				blas_bri.primitiveOffset = 0;
				blas_bri.firstVertex = 0;
				blas_bri.transformOffset = 0;

				tlasbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::UPDATE;
				tlasbiGBI.srcAccelerationStructure = tlas;
				tlasbiGBI.dstAccelerationStructure = tlas;
				tlas_asbi = AccelerationStructureBuildInfo::Create(&tlasbiGBI);

				tlas_bri.primitiveCount = tlasbiGBI.maxPrimitiveCounts[0];
				tlas_bri.primitiveOffset = 0;
				tlas_bri.firstVertex = 0;
				tlas_bri.transformOffset = 0;

				cmdBuffer->Reset(2, false);
				cmdBuffer->Begin(2, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
				cmdBuffer->BuildAccelerationStructures(2, { blas_asbi, tlas_asbi }, { { blas_bri }, { tlas_bri } });
				cmdBuffer->End(2);

				Fence::CreateInfo asUpdateFenceCI = { "AccelStructUpdateFence", device, false, UINT64_MAX };
				FenceRef asUpdateFence = Fence::Create(&asUpdateFenceCI);
				CommandBuffer::SubmitInfo updateSI = { { 2 }, {}, {}, {}, {}, {} };
				cmdBuffer->Submit({ updateSI }, asUpdateFence);
				asUpdateFence->Wait();
			}

			frameIndex = (frameIndex + 1) % 2;
			frameCount++;
		}
	}
	device->DeviceWaitIdle();
}