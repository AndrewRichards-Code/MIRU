#include <miru_core.h>

using namespace miru;
using namespace crossplatform;

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
    cmdBufferCI.commandBufferCount = swapchainCI.swapchainCount;
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
