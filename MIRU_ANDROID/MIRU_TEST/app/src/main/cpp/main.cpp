#include <miru_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../dep/STBI/stb_image.h"

using namespace miru;
using namespace crossplatform;

std::vector<char> load_file_bin(const char* filepath, android_app* app)
{
    AAsset* file = AAssetManager_open(app->activity->assetManager, filepath, AASSET_MODE_BUFFER);
    size_t fileLength = static_cast<size_t >(AAsset_getLength(file));
    std::vector<char> file_bin(fileLength);
    AAsset_read(file, file_bin.data(), file_bin.size());
    return file_bin;
}

extern "C" {

//Forward Declarations
ANativeWindow* gp_AndroidWindow = nullptr;
bool g_CloseWindow = false;
static void handle_cmd(android_app* app, int32_t cmd)
{
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            gp_AndroidWindow = app->window;
            g_CloseWindow = false;
            break;
        case APP_CMD_TERM_WINDOW:
            g_CloseWindow = true;
            break;
        default:
            __android_log_write(ANDROID_LOG_INFO, "MIRU_TEST", "Event not handled");
    }
}


void android_main(struct android_app* app) {

    app->onAppCmd = handle_cmd;
    int events;
    android_poll_source* source;
    while(gp_AndroidWindow == nullptr) {
        if (ALooper_pollAll(1, nullptr, &events, (void **) &source) >= 0)
        {
            if (source != NULL)
                source->process(app, source);
        }
    }

    GraphicsAPI::SetAPI(GraphicsAPI::API::VULKAN);
    GraphicsAPI::AllowSetName();

    Context::CreateInfo contextCI;
    contextCI.api_version_major = 1;
    contextCI.api_version_minor = 0;
    contextCI.applicationName = "MIRU_TEST";
    contextCI.instanceLayers = { "VK_LAYER_LUNARG_standard_validation" };
    contextCI.instanceExtensions = { "VK_KHR_surface", "VK_KHR_android_surface" };
    contextCI.deviceLayers = { "VK_LAYER_LUNARG_standard_validation" };
    contextCI.deviceExtensions = { "VK_KHR_swapchain" };
    contextCI.deviceDebugName = "GPU Device";
    Ref<Context> context = Context::Create(&contextCI);

    Swapchain::CreateInfo swapchainCI;
    swapchainCI.debugName = "Swapchain";
    swapchainCI.pContext = context;
    swapchainCI.pWindow = gp_AndroidWindow;
    swapchainCI.width = 0;
    swapchainCI.height = 0;
    swapchainCI.swapchainCount = 2;
    swapchainCI.vSync = true;
    Ref<Swapchain> swapchain = Swapchain::Create(&swapchainCI);
    uint32_t width = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().pImage->GetCreateInfo().width;
    uint32_t height = swapchain->m_SwapchainImageViews[0]->GetCreateInfo().pImage->GetCreateInfo().height;

    Shader::CreateInfo shaderCI;
    shaderCI.debugName = "VS_Basic";
    shaderCI.device = context->GetDevice();
    shaderCI.stage = Shader::StageBit::VERTEX_BIT;
    shaderCI.entryPoint = "main";
    shaderCI.binaryFilepath = nullptr;
    shaderCI.binaryCode = load_file_bin("bin/basic.vert.spv", app);
    shaderCI.recompileArguments = {0};
    Ref<Shader> vertexShader = Shader::Create(&shaderCI);
    shaderCI.debugName = "FS_Basic";
    shaderCI.stage = Shader::StageBit::FRAGMENT_BIT;
    shaderCI.binaryCode = load_file_bin("bin/basic.frag.spv", app);
    Ref<Shader> fragmentShader = Shader::Create(&shaderCI);

    CommandPool::CreateInfo cmdPoolCI;
    cmdPoolCI.debugName = "CmdPool";
    cmdPoolCI.pContext = context;
    cmdPoolCI.flags =CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
    cmdPoolCI.queueFamilyIndex = 0;
    Ref<CommandPool> cmdPool = CommandPool::Create(&cmdPoolCI);

    CommandBuffer::CreateInfo cmdBufferCI;
    cmdBufferCI.debugName = "PresentCmdBuffers";
    cmdBufferCI.pCommandPool = cmdPool;
    cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
    cmdBufferCI.commandBufferCount = swapchainCI.swapchainCount + 1;
    Ref<CommandBuffer> cmdBuffer = CommandBuffer::Create(&cmdBufferCI);

    MemoryBlock::CreateInfo mbCI;
    mbCI.debugName = "UMA_MB_0";
    mbCI.pContext = context;
    mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_64MB;
    mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT
            | MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT
            | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
    Ref<MemoryBlock> uma_mb_0 = MemoryBlock::Create(&mbCI);

    float vertices[24] =
            {
                    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
                    +0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
                    +0.5f, +0.5f, 0.0f, 1.0f, 1.0f, 0.0f,
                    -0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            };
    uint32_t indices[6] = { 0,1,2,2,3,0 };

    Buffer::CreateInfo vbCI;
    vbCI.debugName = "Quad_VB";
    vbCI.device = context->GetDevice();
    vbCI.usage = Buffer::UsageBit::VERTEX;
    vbCI.size = sizeof(vertices);
    vbCI.data = (void*)vertices;
    vbCI.pMemoryBlock = uma_mb_0;
    Ref<Buffer> vb = Buffer::Create(&vbCI);

    BufferView::CreateInfo vbvCI;
    vbvCI.debugName = "Quad_VBV";
    vbvCI.device = context->GetDevice();
    vbvCI.type = BufferView::Type::VERTEX;
    vbvCI.pBuffer = vb;
    vbvCI.offset = 0;
    vbvCI.size = vbCI.size;
    vbvCI.stride = 6 * sizeof(float);
    Ref<BufferView> vbv = BufferView::Create(&vbvCI);

    Buffer::CreateInfo ibCI;
    ibCI.debugName = "Quad_IB";
    ibCI.device = context->GetDevice();
    ibCI.usage = Buffer::UsageBit::INDEX;
    ibCI.size = sizeof(indices);
    ibCI.data = (void*)indices;
    ibCI.pMemoryBlock = uma_mb_0;
    Ref<Buffer> ib = Buffer::Create(&ibCI);

    BufferView::CreateInfo ibvCI;
    ibvCI.debugName = "Quad_IBV";
    ibvCI.device = context->GetDevice();
    ibvCI.type = BufferView::Type::INDEX;
    ibvCI.pBuffer = ib;
    ibvCI.offset = 0;
    ibvCI.size = ibCI.size;
    ibvCI.stride = sizeof(uint32_t);
    Ref<BufferView> ibv = BufferView::Create(&ibvCI);

    int logo_width, logo_heigth, logo_channels;
    std::vector<char> logo_png = load_file_bin("img/logo.png", app);
    stbi_uc* logo_data = stbi_load_from_memory((stbi_uc*)logo_png.data(), (int)logo_png.size(), &logo_width, &logo_heigth, &logo_channels, 4);

    Buffer::CreateInfo logoUploadCI;
    logoUploadCI.debugName = "LogoUpload";
    logoUploadCI.device = context->GetDevice();
    logoUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
    logoUploadCI.size = (size_t)(logo_width * logo_heigth *logo_channels);
    logoUploadCI.data = (void*)logo_data;
    logoUploadCI.pMemoryBlock = uma_mb_0;
    Ref<Buffer> logoUpload = Buffer::Create(&logoUploadCI);

    Image::CreateInfo logoCI;
    logoCI.debugName = "Logo";
    logoCI.device = context->GetDevice();
    logoCI.type = Image::Type::TYPE_2D;
    logoCI.format = Image::Format::R8G8B8A8_UNORM;
    logoCI.width = (uint32_t)logo_width;
    logoCI.height = (uint32_t)logo_heigth;
    logoCI.depth = 1;
    logoCI.mipLevels = 1;
    logoCI.arrayLayers = 1;
    logoCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
    logoCI.usage = Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::SAMPLED_BIT;;
    logoCI.layout = Image::Layout::UNKNOWN;
    logoCI.size = 0;
    logoCI.data = nullptr;
    logoCI.pMemoryBlock = uma_mb_0;
    Ref<Image> logo = Image::Create(&logoCI);

    //Image Transfer and Layout Transition
    {
        cmdBuffer->Begin(2,CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

        Barrier::CreateInfo bCI;
        bCI.type = Barrier::Type::IMAGE;
        bCI.srcAccess = Barrier::AccessBit::NONE;
        bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
        bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
        bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
        bCI.pImage = logo;
        bCI.oldLayout = Image::Layout::UNKNOWN;
        bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
        bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
        Ref<Barrier> b0 = Barrier::Create(&bCI);
        cmdBuffer->PipelineBarrier(2, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b0 });
        cmdBuffer->CopyBufferToImage(2, logoUpload, logo, Image::Layout::TRANSFER_DST_OPTIMAL,
                { {0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0,0,0}, {logoCI.width, logoCI.height, logoCI.depth}} });

        bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
        bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
        bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
        bCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
        Ref<Barrier> b1 = Barrier::Create(&bCI);
        cmdBuffer->PipelineBarrier(2, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::FRAGMENT_SHADER_BIT, DependencyBit::NONE_BIT, { b1 });

        cmdBuffer->End(2);
    }
    cmdBuffer->Submit({ 2 }, {}, {}, PipelineStageBit::TRANSFER_BIT, nullptr);

    while (!g_CloseWindow)
    {
        if(ALooper_pollAll(1, nullptr, &events, (void**)&source) >=0)
        {
            if (source != NULL)
                source->process(app, source);
        }

        __android_log_write(ANDROID_LOG_INFO, "MIRU_TEST", std::to_string(width).c_str());
        __android_log_write(ANDROID_LOG_INFO, "MIRU_TEST", std::to_string(height).c_str());
    }
    return;
}

}
