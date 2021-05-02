#include "miru_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../dep/STBI/stb_image.h"

#undef far
#undef near

using namespace miru;
using namespace crossplatform;

class Vec3
{
public:
	float x, y, z;
};
class Vec4
{
public:
	float x, y, z, w;

	Vec4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w) {}

	Vec4 operator+ (const Vec4& other) const
	{
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4 operator* (float a) const
	{
		return Vec4(a * x, a * y, a * z, a * w);
	}
};
class Mat4
{
public:
	float a, b, c, d, 
		e, f, g, h, 
		i, j, k, l, 
		m, n, o, p;

	Mat4()
		:a(0), b(0), c(0), d(0), e(0), f(0), g(0), h(0),
		i(0), j(0), k(0), l(0), m(0), n(0), o(0), p(0) {}

	Mat4(float a, float b, float c, float d, float e, float f, float g, float h,
		float i, float j, float k, float l, float m, float n, float o, float p)
		: a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h),
		i(i), j(j), k(k), l(l), m(m), n(n), o(o), p(p) {}

	Mat4(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d)
		: a(a.x), b(a.y), c(a.z), d(a.w), e(b.x), f(b.y), g(b.z), h(b.w),
		i(c.x), j(c.y), k(c.z), l(c.w), m(d.x), n(d.y), o(d.z), p(d.w) {}

	Mat4(float diagonal)
		: a(diagonal), b(0), c(0), d(0), e(0), f(diagonal), g(0), h(0),
		i(0), j(0), k(diagonal), l(0), m(0), n(0), o(0), p(diagonal) {}

	static Mat4 Identity()
	{
		return Mat4(1);
	}

	void Transpose()
	{
		a = a;
		f = f;
		k = k;
		p = p;

		float temp_b = b;
		float temp_c = c;
		float temp_d = d;
		float temp_g = g;
		float temp_h = h;
		float temp_l = l;

		b = e;
		c = i;
		d = m;
		g = j;
		h = n;
		l = o;

		e = temp_b;
		i = temp_c;
		m = temp_d;
		j = temp_g;
		n = temp_h;
		o = temp_l;
	}

	static Mat4 Perspective(double fov, float aspectRatio, float near, float far)
	{
		return Mat4((1 / (aspectRatio * static_cast<float>(tan(fov / 2)))), (0), (0), (0),
			(0), (1 / static_cast<float>(tan(fov / 2))), (0), (0),
			(0), (0), -((far + near) / (far - near)), -((2 * far * near) / (far - near)),
			(0), (0), (-1), (0));
	}

	static Mat4 Translation(const Vec3& translation)
	{
		Mat4 result(1);
		result.d = translation.x;
		result.h = translation.y;
		result.l = translation.z;
		return result;
	}

	static Mat4 Rotation(double angle, const Vec3& axis)
	{
		Mat4 result(1);
		float c_angle = static_cast<float>(cos(angle));
		float s_angle = static_cast<float>(sin(angle));
		float omcos = static_cast<float>(1 - c_angle);

		float x = axis.x;
		float y = axis.y;
		float z = axis.z;

		result.a = x * x * omcos + c_angle;
		result.e = x * y * omcos + z * s_angle;
		result.i = x * z * omcos - y * s_angle;
		result.m = 0;

		result.b = y * x * omcos - z * s_angle;
		result.f = y * y * omcos + c_angle;
		result.j = y * z * omcos + x * s_angle;
		result.n = 0;

		result.c = z * x * omcos + y * s_angle;
		result.g = z * y * omcos - x * s_angle;
		result.k = z * z * omcos + c_angle;
		result.o = 0;

		result.d = 0;
		result.h = 0;
		result.l = 0;
		result.p = 1;

		return result;
	}

	static Mat4 Scale(const Vec3& scale)
	{
		Mat4 result(1);
		result.a = scale.x;
		result.f = scale.y;
		result.k = scale.z;
		return result;
	}

	Mat4 operator*(const Mat4& input) const
	{
		Vec4 input_i(input.a, input.e, input.i, input.m);
		Vec4 input_j(input.b, input.f, input.j, input.n);
		Vec4 input_k(input.c, input.g, input.k, input.o);
		Vec4 input_l(input.d, input.h, input.l, input.p);
		Vec4 output_i = *this * input_i;
		Vec4 output_j = *this * input_j;
		Vec4 output_k = *this * input_k;
		Vec4 output_l = *this * input_l;
		Mat4 output(output_i, output_j, output_k, output_l);
		output.Transpose();
		return output;
	}

	Vec4 operator*(const Vec4& input) const
	{
		float x = input.x;
		float y = input.y;
		float z = input.z;
		float w = input.w;
		Vec4 transform_i(a, e, i, m);
		Vec4 transform_j(b, f, j, n);
		Vec4 transform_k(c, g, k, o);
		Vec4 transform_l(d, h, l, p);
		Vec4 output(transform_i * x + transform_j * y + transform_k * z + transform_l * w);
		return output;
	}
};

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
	GraphicsAPI::SetAPI(GraphicsAPI::API::D3D12);
	//GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
	GraphicsAPI::AllowSetName();
	//GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::RENDER_DOC);

	MIRU_CPU_PROFILE_BEGIN_SESSION("miru_profile_result.txt");

	Context::CreateInfo contextCI;
	contextCI.api_version_major = GraphicsAPI::IsD3D12() ? 12 : 1;
	contextCI.api_version_minor = GraphicsAPI::IsD3D12() ? 1 : 2;
	contextCI.applicationName = "MIRU_TEST";
	contextCI.instanceLayers = { "VK_LAYER_KHRONOS_validation" };
	contextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_win32_surface" };
	contextCI.deviceLayers = { "VK_LAYER_KHRONOS_validation" };
	contextCI.deviceExtensions = { "VK_KHR_swapchain",
		"VK_KHR_acceleration_structure", "VK_KHR_ray_tracing_pipeline",
		"VK_KHR_deferred_host_operations" };
	contextCI.deviceDebugName = "GPU Device";
	Ref<Context> context = Context::Create(&contextCI);

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
	shaderCI.debugName = "Basic: Vertex Shader Module";
	shaderCI.device = context->GetDevice();
	shaderCI.stageAndEntryPoints = { {Shader::StageBit::VERTEX_BIT, "vs_main"} };
	shaderCI.binaryFilepath = "res/bin/basic_vs_5_1_vs_main.spv";
	shaderCI.binaryCode = {};
	#if _DEBUG
	bool debug = true;
	#else
	bool debug = false;
	#endif
	shaderCI.recompileArguments = {
		debug ? "../MIRU_SHADER_COMPILER/exe/x64/Debug" : "../MIRU_SHADER_COMPILER/exe/x64/Release",
		"res/shaders/basic.hlsl",
		"res/bin",
		{"../MIRU_SHADER_COMPILER/shaders/includes"},
		"vs_main",
		"vs_5_1",
		{},
		true,
		true,
		"",
		"",
		false,
		false 
	};
	Ref<Shader> vertexShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "Basic: Fragment Shader Module";
	shaderCI.stageAndEntryPoints = { { Shader::StageBit::PIXEL_BIT, "ps_main"} };
	shaderCI.binaryFilepath = "res/bin/basic_ps_5_1_ps_main.spv";
	shaderCI.recompileArguments.entryPoint = "ps_main";
	shaderCI.recompileArguments.shaderModel = "ps_5_1";
	Ref<Shader> fragmentShader = Shader::Create(&shaderCI);
	shaderCI.debugName = "RayTracing: Library Shader Module";
	shaderCI.stageAndEntryPoints = { 
		{ Shader::StageBit::RAYGEN_BIT, "ray_generation_main"},
		{ Shader::StageBit::ANY_HIT_BIT, "any_hit_main"},
		{ Shader::StageBit::CLOSEST_HIT_BIT, "closest_hit_main"},
		{ Shader::StageBit::MISS_BIT, "miss_main"},
	};
	shaderCI.binaryFilepath = "res/bin/RayTracing_lib_6_3.spv";
	shaderCI.recompileArguments.entryPoint = "";
	shaderCI.recompileArguments.shaderModel = "lib_6_3";
	Ref<Shader> raytracingShader = Shader::Create(&shaderCI);

	CommandPool::CreateInfo cmdPoolCI;
	cmdPoolCI.debugName = "CmdPool";
	cmdPoolCI.pContext = context;
	cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	cmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
	Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);
	cmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
	Ref<CommandPool> cmdCopyPool = CommandPool::Create(&cmdPoolCI);

	CommandBuffer::CreateInfo cmdBufferCI, cmdCopyBufferCI;
	cmdBufferCI.debugName = "CmdBuffer";
	cmdBufferCI.pCommandPool = cmdPool;
	cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdBufferCI.commandBufferCount = 3;
	cmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);
	cmdCopyBufferCI.debugName = "CmdCopyBuffer";
	cmdCopyBufferCI.pCommandPool = cmdCopyPool;
	cmdCopyBufferCI.level = CommandBuffer::Level::PRIMARY;
	cmdCopyBufferCI.commandBufferCount = 1;
	cmdCopyBufferCI.allocateNewCommandPoolPerBuffer = false;
	Ref<CommandBuffer> cmdCopyBuffer = CommandBuffer::Create(&cmdCopyBufferCI);

	Allocator::CreateInfo allocCI;
	allocCI.debugName = "CPU_ALLOC_0";
	allocCI.pContext = context;
	allocCI.blockSize = Allocator::BlockSize::BLOCK_SIZE_64MB;
	allocCI.properties = Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT;
	Ref<Allocator> cpu_alloc_0 = Allocator::Create(&allocCI);
	allocCI.debugName = "GPU_ALLOC_0";
	allocCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	Ref<Allocator> gpu_alloc_0 = Allocator::Create(&allocCI);

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
	uint8_t* imageData = stbi_load("../logo.png", &img_width, &img_height, &bpp, 4);

	Buffer::CreateInfo verticesBufferCI;
	verticesBufferCI.debugName = "Vertices Buffer";
	verticesBufferCI.device = context->GetDevice();
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	verticesBufferCI.size = sizeof(vertices);
	verticesBufferCI.data = vertices;
	verticesBufferCI.pAllocator = cpu_alloc_0;
	Ref<Buffer> c_vb = Buffer::Create(&verticesBufferCI);
	verticesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT;
	verticesBufferCI.data = nullptr;
	verticesBufferCI.pAllocator = gpu_alloc_0;
	Ref<Buffer> g_vb = Buffer::Create(&verticesBufferCI);

	Buffer::CreateInfo indicesBufferCI;
	indicesBufferCI.debugName = "Indices Buffer";
	indicesBufferCI.device = context->GetDevice();
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT | Buffer::UsageBit::ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT;
	indicesBufferCI.size = sizeof(indices);
	indicesBufferCI.data = indices;
	indicesBufferCI.pAllocator = cpu_alloc_0;
	Ref<Buffer> c_ib = Buffer::Create(&indicesBufferCI);
	indicesBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	indicesBufferCI.data = nullptr;
	indicesBufferCI.pAllocator = gpu_alloc_0;
	Ref<Buffer> g_ib = Buffer::Create(&indicesBufferCI);

	Buffer::CreateInfo imageBufferCI;
	imageBufferCI.debugName = "MIRU logo upload buffer";
	imageBufferCI.device = context->GetDevice();
	imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	imageBufferCI.size = img_width * img_height * 4;
	imageBufferCI.data = imageData;
	imageBufferCI.pAllocator = cpu_alloc_0;
	Ref<Buffer> c_imageBuffer = Buffer::Create(&imageBufferCI);

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
	Ref<Image> image = Image::Create(&imageCI);

	Mat4 proj = Mat4::Identity();
	Mat4 view = Mat4::Identity();
	Mat4 modl = Mat4::Identity();
	
	float ubData[2 * sizeof(Mat4)];
	memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
	memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

	Buffer::CreateInfo ubCI;
	ubCI.debugName = "Camera UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = 2 * sizeof(Mat4);
	ubCI.data = ubData;
	ubCI.pAllocator = cpu_alloc_0;
	Ref<Buffer> ub1 = Buffer::Create(&ubCI);
	ubCI.debugName = "Model UB";
	ubCI.device = context->GetDevice();
	ubCI.usage = Buffer::UsageBit::UNIFORM_BIT;
	ubCI.size = sizeof(Mat4);
	ubCI.data = &modl.a;
	ubCI.pAllocator = cpu_alloc_0;
	Ref<Buffer> ub2 = Buffer::Create(&ubCI);

	BufferView::CreateInfo ubViewCamCI;
	ubViewCamCI.debugName = "Camera UBView";
	ubViewCamCI.device = context->GetDevice();
	ubViewCamCI.type = BufferView::Type::UNIFORM;
	ubViewCamCI.pBuffer = ub1;
	ubViewCamCI.offset = 0;
	ubViewCamCI.size = 2 * sizeof(Mat4);
	ubViewCamCI.stride = 0;
	Ref<BufferView> ubViewCam = BufferView::Create(&ubViewCamCI);

	BufferView::CreateInfo ubViewMdlCI;
	ubViewMdlCI.debugName = "Model UBView";
	ubViewMdlCI.device = context->GetDevice();
	ubViewMdlCI.type = BufferView::Type::UNIFORM;
	ubViewMdlCI.pBuffer = ub2;
	ubViewMdlCI.offset = 0;
	ubViewMdlCI.size = sizeof(Mat4);
	ubViewMdlCI.stride = 0;
	Ref<BufferView> ubViewMdl = BufferView::Create(&ubViewMdlCI);

#if 1
	AccelerationStructureBuildInfo::BuildGeometryInfo asbiGBI;
	asbiGBI.device = context->GetDevice();
	asbiGBI.type = AccelerationStructureBuildInfo::BuildGeometryInfo::Type::BOTTOM_LEVEL;
	asbiGBI.flags = AccelerationStructureBuildInfo::BuildGeometryInfo::FlagBit::PREFER_FAST_TRACE_BIT;
	asbiGBI.mode = AccelerationStructureBuildInfo::BuildGeometryInfo::Mode::BUILD;
	asbiGBI.srcAccelerationStructure = nullptr;
	asbiGBI.dstAccelerationStructure = nullptr;
	asbiGBI.geometries = {
		{
			AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::Type::TRIANGLES,
			{
				VertexType::VEC4,
				GetBufferDeviceAddress(context->GetDevice(), c_vb),
				static_cast<uint64_t>(4 * sizeof(float)),
				_countof(vertices) / 4,
				IndexType::UINT32,
				GetBufferDeviceAddress(context->GetDevice(), c_ib),
				_countof(indices),
				GetBufferDeviceAddress(context->GetDevice(), ub2)
			},
			AccelerationStructureBuildInfo::BuildGeometryInfo::Geometry::FlagBit::OPAQUE_BIT
		}
	};
	asbiGBI.scratchData = MIRU_NULL_DEVICE_OR_HOST_ADDRESS;
	asbiGBI.buildType = AccelerationStructureBuildInfo::BuildGeometryInfo::BuildType::DEVICE;
	asbiGBI.maxPrimitiveCounts = _countof(indices) / 3;
	Ref<AccelerationStructureBuildInfo> asbi = AccelerationStructureBuildInfo::Create(&asbiGBI);

	Buffer::CreateInfo asBufferCI;
	asBufferCI.debugName = "BLASBuffer";
	asBufferCI.device = context->GetDevice();
	asBufferCI.usage = Buffer::UsageBit::ACCELERATION_STRUCTURE_STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	asBufferCI.size = asbi->GetBuildSizesInfo().accelerationStructureSize;
	asBufferCI.data = nullptr;
	asBufferCI.pAllocator = gpu_alloc_0;
	Ref<Buffer> asBuffer = Buffer::Create(&asBufferCI);

	Buffer::CreateInfo scratchBufferCI;
	scratchBufferCI.debugName = "ScratchBuffer";
	scratchBufferCI.device = context->GetDevice();
	scratchBufferCI.usage = Buffer::UsageBit::STORAGE_BIT | Buffer::UsageBit::SHADER_DEVICE_ADDRESS_BIT;
	scratchBufferCI.size = asbi->GetBuildSizesInfo().buildScratchSize;
	scratchBufferCI.data = nullptr;
	scratchBufferCI.pAllocator = gpu_alloc_0;
	Ref<Buffer> scratchBuffer = Buffer::Create(&scratchBufferCI);

	AccelerationStructure::CreateInfo asCI;
	asCI.debugName = "BLAS";
	asCI.device = context->GetDevice();
	asCI.flags = AccelerationStructure::FlagBit::NONE_BIT;
	asCI.buffer = asBuffer;
	asCI.offset = 0;
	asCI.size = asBufferCI.size;
	asCI.type = AccelerationStructure::Type::BOTTOM_LEVEL;
	asCI.deviceAddress = MIRU_NULL_DEVICE_ADDRESS;
	Ref<AccelerationStructure> blas = AccelerationStructure::Create(&asCI);

	asbiGBI.dstAccelerationStructure = blas;
	asbiGBI.scratchData.deviceAddress = GetBufferDeviceAddress(context->GetDevice(), scratchBuffer);
	asbi = AccelerationStructureBuildInfo::Create(&asbiGBI);

	AccelerationStructureBuildInfo::BuildRangeInfo bri;
	bri.primitiveCount = asbiGBI.maxPrimitiveCounts;
	bri.primitiveOffset = 0;
	bri.firstVertex = 0;
	bri.transformOffset = 0;
#endif

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

		cmdBuffer->BuildAccelerationStructure(2, { asbi }, { { bri } });

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
	Ref<BufferView> vbv = BufferView::Create(&vbViewCI);

	BufferView::CreateInfo ibViewCI;
	ibViewCI.debugName = "IndicesBufferView";
	ibViewCI.device = context->GetDevice();
	ibViewCI.type = BufferView::Type::INDEX;
	ibViewCI.pBuffer = g_ib;
	ibViewCI.offset = 0;
	ibViewCI.size = sizeof(indices);
	ibViewCI.stride = sizeof(uint32_t);
	Ref<BufferView> ibv = BufferView::Create(&ibViewCI);

	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "MIRU logo ImageView";
	imageViewCI.device = context->GetDevice();
	imageViewCI.pImage = image;
	imageViewCI.viewType = Image::Type::TYPE_CUBE;
	imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 6 };
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
	Ref<Image> colourImage = Image::Create(&colourCI);

	ImageView::CreateInfo colourImageViewCI;
	colourImageViewCI.debugName = "Colour ImageView";
	colourImageViewCI.device = context->GetDevice();
	colourImageViewCI.pImage = colourImage;
	colourImageViewCI.viewType = Image::Type::TYPE_2D;
	colourImageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
	Ref<ImageView> colourImageView = ImageView::Create(&colourImageViewCI);

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
	Ref<Image> depthImage = Image::Create(&depthCI);
	
	ImageView::CreateInfo depthImageViewCI;
	depthImageViewCI.debugName = "Depth ImageView";
	depthImageViewCI.device = context->GetDevice();
	depthImageViewCI.pImage = depthImage;
	depthImageViewCI.viewType = Image::Type::TYPE_2D;
	depthImageViewCI.subresourceRange = { Image::AspectBit::DEPTH_BIT, 0, 1, 0, 1 };
	Ref<ImageView> depthImageView = ImageView::Create(&depthImageViewCI);

	DescriptorPool::CreateInfo descriptorPoolCI;
	descriptorPoolCI.debugName = "Basic: Descriptor Pool";
	descriptorPoolCI.device = context->GetDevice();
	descriptorPoolCI.poolSizes = { {DescriptorType::COMBINED_IMAGE_SAMPLER, 1},  {DescriptorType::UNIFORM_BUFFER, 2} };
	descriptorPoolCI.maxSets = 2;
	Ref<DescriptorPool> descriptorPool = DescriptorPool::Create(&descriptorPoolCI);
	DescriptorSetLayout::CreateInfo setLayoutCI;
	setLayoutCI.debugName = "Basic: DescSetLayout1";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT } };
	Ref<DescriptorSetLayout> setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "Basic: DescSetLayout2";
	setLayoutCI.descriptorSetLayoutBinding = { 
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT },
		{1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT }
	};
	Ref<DescriptorSetLayout> setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
	DescriptorSet::CreateInfo descriptorSetCI;
	descriptorSetCI.debugName = "Basic: Descriptor Set";
	descriptorSetCI.pDescriptorPool = descriptorPool;
	descriptorSetCI.pDescriptorSetLayouts = { setLayout1, setLayout2 };
	Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(&descriptorSetCI);
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
		GraphicsAPI::IsD3D12()?Image::Layout::PRESENT_SRC : Image::Layout::UNKNOWN,
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
	Ref<RenderPass> renderPass = RenderPass::Create(&renderPassCI);

	Pipeline::CreateInfo pCI;
	pCI.debugName = "Basic";
	pCI.device = context->GetDevice();
	pCI.type = PipelineType::GRAPHICS;
	pCI.shaders = { vertexShader, fragmentShader };
	pCI.vertexInputState.vertexInputBindingDescriptions = { {0, sizeof(vertices)/8, VertexInputRate::VERTEX} };
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
	Ref<Pipeline> pipeline = Pipeline::Create(&pCI);

	setLayoutCI.debugName = "RayTracing: DescSetLayout1";
	setLayoutCI.device = context->GetDevice();
	setLayoutCI.descriptorSetLayoutBinding = { 
		{0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT }, 
		{1, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT } };
	Ref<DescriptorSetLayout> setLayout1RT = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "RayTracing: DescSetLayout2";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::STORAGE_IMAGE, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT },
		{1, DescriptorType::ACCELERATION_STRUCTURE, 1, Shader::StageBit::RAYGEN_BIT | Shader::StageBit::ANY_HIT_BIT | Shader::StageBit::CLOSEST_HIT_BIT | Shader::StageBit::MISS_BIT }
	};
	Ref<DescriptorSetLayout> setLayout2RT = DescriptorSetLayout::Create(&setLayoutCI);
	setLayoutCI.debugName = "RayTracing: DescSetLayout3";
	setLayoutCI.descriptorSetLayoutBinding = {
		{0, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::CLOSEST_HIT_BIT }
	};
	Ref<DescriptorSetLayout> setLayout3RT = DescriptorSetLayout::Create(&setLayoutCI);

	Pipeline::CreateInfo raytracingPipelineCI;
	raytracingPipelineCI.debugName = "Ray Tracing Pipeline";
	raytracingPipelineCI.device = context->GetDevice();
	raytracingPipelineCI.type = PipelineType::RAY_TRACING;
	raytracingPipelineCI.shaders = { raytracingShader };
	raytracingPipelineCI.dynamicStates = {};
	raytracingPipelineCI.shaderGroupInfos = {
		{ ShaderGroupType::GENERAL, 0, MIRU_SHADER_UNUSED, MIRU_SHADER_UNUSED, MIRU_SHADER_UNUSED },
		{ ShaderGroupType::TRIANGLES_HIT_GROUP, MIRU_SHADER_UNUSED, 1, 2, MIRU_SHADER_UNUSED, { {setLayout3RT}, {} }, 2 },
		{ ShaderGroupType::GENERAL, 3, MIRU_SHADER_UNUSED, MIRU_SHADER_UNUSED, MIRU_SHADER_UNUSED },
	};
	raytracingPipelineCI.rayTracingInfo = { 1, 16, 8, cpu_alloc_0 };
	raytracingPipelineCI.layout = { {setLayout1RT, setLayout2RT}, {} };
	Ref<Pipeline> raytracingPipeline = Pipeline::Create(&raytracingPipelineCI);

	Framebuffer::CreateInfo framebufferCI_0, framebufferCI_1;
	framebufferCI_0.debugName = "Framebuffer0";
	framebufferCI_0.device = context->GetDevice();
	framebufferCI_0.renderPass = renderPass;
	framebufferCI_0.attachments = { colourImageView, depthImageView, swapchain->m_SwapchainImageViews[0] };
	framebufferCI_0.width = width;
	framebufferCI_0.height = height;
	framebufferCI_0.layers = 1;
	Ref<Framebuffer> framebuffer0 = Framebuffer::Create(&framebufferCI_0);
	framebufferCI_1.debugName = "Framebuffer1";
	framebufferCI_1.device = context->GetDevice();
	framebufferCI_1.renderPass = renderPass;
	framebufferCI_1.attachments = { colourImageView, depthImageView, swapchain->m_SwapchainImageViews[1] };
	framebufferCI_1.width = width;
	framebufferCI_1.height = height;
	framebufferCI_1.layers = 1;
	Ref<Framebuffer> framebuffer1 = Framebuffer::Create(&framebufferCI_1);

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
			vertexShader->Recompile();
			fragmentShader->Recompile();

			pCI.shaders = { vertexShader, fragmentShader };
			pipeline = Pipeline::Create(&pCI);

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			shaderRecompile = false;
			frameIndex = 0;
		}
		if (swapchain->m_Resized)
		{
			swapchain->Resize(width, height);
			
			pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
			pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
			pipeline = Pipeline::Create(&pCI);

			colourCI.width = width;
			colourCI.height = height;
			colourImage = Image::Create(&colourCI);
			colourImageViewCI.pImage = colourImage;
			colourImageView = ImageView::Create(&colourImageViewCI);

			depthCI.width = width;
			depthCI.height = height;
			depthImage = Image::Create(&depthCI);
			depthImageViewCI.pImage = depthImage;
			depthImageView = ImageView::Create(&depthImageViewCI);

			framebufferCI_0.attachments = { colourImageView, depthImageView, swapchain->m_SwapchainImageViews[0] };
			framebufferCI_0.width = width;
			framebufferCI_0.height = height;
			framebuffer0 = Framebuffer::Create(&framebufferCI_0);

			framebufferCI_1.attachments = { colourImageView, depthImageView, swapchain->m_SwapchainImageViews[1] };
			framebufferCI_1.width = width;
			framebufferCI_1.height = height;
			framebuffer1 = Framebuffer::Create(&framebufferCI_1);

			cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

			draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
			acquire = { Semaphore::Create(&acquireSemaphoreCI), Semaphore::Create(&acquireSemaphoreCI) };
			submit = { Semaphore::Create(&submitSemaphoreCI), Semaphore::Create(&submitSemaphoreCI) };

			swapchain->m_Resized = false;
			frameIndex = 0;
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

			proj = Mat4::Perspective(3.14159/2.0, float(width)/float(height), 0.1f, 100.0f);
			if (GraphicsAPI::IsVulkan())
				proj.k *= -1;
			modl = Mat4::Translation({ 0.0f, 0.0f, -1.5f })
				//* translate(mat4(1.0f), { float(var_x)/10.0f, float(var_y)/10.0f, float(var_z)/10.0f})
				* Mat4::Rotation((var_x * 5.0f) * 3.14159/180.0, { 0, 1, 0 })
				* Mat4::Rotation((var_y * 5.0f) * 3.14159/180.0, { 1, 0, 0 })
				* Mat4::Rotation((var_z * 5.0f) * 3.14159/180.0, { 0, 0, 1 });

			memcpy(ubData + 0 * 16, &proj.a, sizeof(Mat4));
			memcpy(ubData + 1 * 16, &view.a, sizeof(Mat4));

			cpu_alloc_0->SubmitData(ub1->GetAllocation(), 2 * sizeof(Mat4), ubData);
			cpu_alloc_0->SubmitData(ub2->GetAllocation(), sizeof(Mat4), (void*)&modl.a);
		}
		cmdBuffer->Present({ 0, 1 }, swapchain, draws, acquire, submit, windowResize);
		frameIndex = (frameIndex + 1) % swapchainCI.swapchainCount;
		frameCount++;
	}
	context->DeviceWaitIdle();
}