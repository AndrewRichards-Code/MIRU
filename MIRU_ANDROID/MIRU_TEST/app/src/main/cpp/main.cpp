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

    while (!g_CloseWindow)
    {
        if(ALooper_pollAll(1, nullptr, &events, (void**)&source) >=0)
        {
            if (source != NULL)
                source->process(app, source);
        }
        int a = 5;
        int b = 3;
        int c = a + b;

        __android_log_write(ANDROID_LOG_INFO, "MIRU_TEST", std::to_string(c).c_str());
    }
    return;
}

}
