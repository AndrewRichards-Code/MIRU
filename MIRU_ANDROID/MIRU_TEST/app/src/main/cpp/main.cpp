#include <miru_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../dep/STBI/stb_image.h"

#include "../dep/glm/glm/glm.hpp"
#include "../dep/glm/glm/gtc/matrix_transform.hpp"

using namespace miru;
using namespace crossplatform;

using namespace glm;

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
    shaderCI.entryPoint = "vs_main";
    shaderCI.binaryFilepath = nullptr;
    shaderCI.binaryCode = load_file_bin("bin/basic_vert_vs_main.spv", app);
    shaderCI.recompileArguments = {0};
    Ref<Shader> vertexShader = Shader::Create(&shaderCI);
    shaderCI.debugName = "FS_Basic";
    shaderCI.stage = Shader::StageBit::FRAGMENT_BIT;
    shaderCI.binaryCode = load_file_bin("bin/basic_frag_ps_main.spv", app);
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
    cmdBuffer->Submit({ 2 }, {}, { PipelineStageBit::TRANSFER_BIT }, {}, nullptr);

    ImageView::CreateInfo logoVICI;
    logoVICI.debugName = "LogoVI";
    logoVICI.device = context->GetDevice();
    logoVICI.pImage = logo;
    logoVICI.subresourceRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
    Ref<ImageView> logoIV = ImageView::Create(&logoVICI);

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

    mat4 proj = identity<mat4>();
    mat4 view = identity<mat4>();
    mat4 modl = identity<mat4>();

    float ubData[2 * sizeof(mat4)];
    memcpy(ubData + 0 * 16, &proj[0][0], sizeof(mat4));
    memcpy(ubData + 1 * 16, &view[0][0], sizeof(mat4));

    Buffer::CreateInfo ubCI;
    ubCI.debugName = "CameraUB";
    ubCI.device = context->GetDevice();
    ubCI.usage = Buffer::UsageBit::UNIFORM;
    ubCI.size = 2 * sizeof(mat4);
    ubCI.data = ubData;
    ubCI.pMemoryBlock = uma_mb_0;
    Ref<Buffer> ub1 = Buffer::Create(&ubCI);
    ubCI.debugName = "ModelUB";
    ubCI.device = context->GetDevice();
    ubCI.usage = Buffer::UsageBit::UNIFORM;
    ubCI.size = sizeof(mat4);
    ubCI.data = &modl[0][0];
    ubCI.pMemoryBlock = uma_mb_0;
    Ref<Buffer> ub2 = Buffer::Create(&ubCI);

    BufferView::CreateInfo ubViewCamCI;
    ubViewCamCI.debugName = "CameraUBView";
    ubViewCamCI.device = context->GetDevice();
    ubViewCamCI.type = BufferView::Type::UNIFORM;
    ubViewCamCI.pBuffer = ub1;
    ubViewCamCI.offset = 0;
    ubViewCamCI.size = 2 * sizeof(mat4);
    ubViewCamCI.stride = 0;
    Ref<BufferView> ubViewCam = BufferView::Create(&ubViewCamCI);
    BufferView::CreateInfo ubViewMdlCI;
    ubViewMdlCI.debugName = "ModelUBView";
    ubViewMdlCI.device = context->GetDevice();
    ubViewMdlCI.type = BufferView::Type::UNIFORM;
    ubViewMdlCI.pBuffer = ub2;
    ubViewMdlCI.offset = 0;
    ubViewMdlCI.size = sizeof(mat4);
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
    depthCI.layout = Image::Layout::UNKNOWN;
    depthCI.size = 0;
    depthCI.data = nullptr;
    depthCI.pMemoryBlock = uma_mb_0;
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
    setLayoutCI.descriptorSetLayoutBinding = { {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT } };
    Ref<DescriptorSetLayout> setLayout1 = DescriptorSetLayout::Create(&setLayoutCI);
    setLayoutCI.descriptorSetLayoutBinding = {
            {1, DescriptorType::COMBINED_IMAGE_SAMPLER, 1, Shader::StageBit::FRAGMENT_BIT },
            {0, DescriptorType::UNIFORM_BUFFER, 1, Shader::StageBit::VERTEX_BIT }
    };
    Ref<DescriptorSetLayout> setLayout2 = DescriptorSetLayout::Create(&setLayoutCI);
    DescriptorSet::CreateInfo descriptorSetCI;
    descriptorSetCI.debugName = "Image Descriptor Set";
    descriptorSetCI.pDescriptorPool = descriptorPool;
    descriptorSetCI.pDescriptorSetLayouts = {setLayout1, setLayout2 };
    Ref<DescriptorSet> descriptorSet = DescriptorSet::Create(&descriptorSetCI);
    descriptorSet->AddBuffer(0, 0, { { ubViewCam } });
    descriptorSet->AddBuffer(1, 0, { { ubViewMdl } });
    descriptorSet->AddImage(1, 1, { { sampler, logoIV, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
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
                    Image::Layout::UNKNOWN,
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
                    PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT,
                    (Barrier::AccessBit)0, Barrier::AccessBit::COLOUR_ATTACHMENT_READ_BIT | Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, DependencyBit::NONE_BIT}
    };
    Ref<RenderPass> renderPass = RenderPass::Create(&renderPassCI);

    Pipeline::CreateInfo pCI;
    pCI.debugName = "Basic";
    pCI.device = context->GetDevice();
    pCI.type = PipelineType::GRAPHICS;
    pCI.shaders = { vertexShader, fragmentShader };
    pCI.vertexInputState.vertexInputBindingDescriptions = { {0, sizeof(vertices)/4, VertexInputRate::VERTEX} };
    pCI.vertexInputState.vertexInputAttributeDescriptions = { {0, 0, VertexType::VEC4, 0, ""}, {1, 0, VertexType::VEC2, 16, ""} };
    pCI.inputAssemblyState = { PrimitiveTopology::TRIANGLE_LIST, false };
    pCI.tessellationState = {};
    pCI.viewportState.viewports = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
    pCI.viewportState.scissors = { {{(int32_t)0, (int32_t)0}, {width, height}} };
    pCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE_BIT, FrontFace::CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f };
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

    Fence::CreateInfo fenceCI;
    fenceCI.debugName = "DrawFence";
    fenceCI.device = context->GetDevice();
    fenceCI.signaled = true;
    fenceCI.timeout = UINT64_MAX;
    std::vector<Ref<Fence>>draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
    Semaphore::CreateInfo semaphoreCI = { "Seamphore", context->GetDevice() };
    std::vector<Ref<Semaphore>>acquire = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };
    std::vector<Ref<Semaphore>>submit = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };

    uint32_t frameIndex = 0;
    uint32_t frameCount = 0;
    float r = 1.00f;
    float g = 0.00f;
    float b = 0.00f;
    float increment = 1.0f / 60.0f;
    int var_x = 0;
    int var_y = 0;
    int var_z = 0;
    bool windowResize;
    while (!g_CloseWindow)
    {
        if(ALooper_pollAll(1, nullptr, &events, (void**)&source) >=0)
        {
            if (source != NULL)
                source->process(app, source);
        }

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

        Image::ClearValue colour, depth;
        colour.colour.float32[0] = r;
        colour.colour.float32[1] = g;
        colour.colour.float32[2] = b;
        colour.colour.float32[3] = 1.0f;
        depth.depthStencil.depth = 0.0f;
        depth.depthStencil.stencil = 0;

        cmdBuffer->Reset(frameIndex, false);
        cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
        cmdBuffer->BeginRenderPass(frameIndex, frameIndex == 0 ? framebuffer0 : framebuffer1, { colour, depth });
        cmdBuffer->BindPipeline(frameIndex, pipeline);
        cmdBuffer->BindDescriptorSets(frameIndex, { descriptorSet }, pipeline);
        cmdBuffer->BindVertexBuffers(frameIndex, { vbv });
        cmdBuffer->BindIndexBuffer(frameIndex, ibv);
        cmdBuffer->DrawIndexed(frameIndex, 6);
        cmdBuffer->EndRenderPass(frameIndex);
        cmdBuffer->End(frameIndex);

        //Update Uniform buffers
        proj = perspectiveFov(radians(90.0f), float(width), float(height), 0.1f, 100.0f);
        if (GraphicsAPI::IsVulkan())
            proj[1][1] *= -1;
        modl = translate(mat4(1.0f),{(float)var_x / 10.0f, (float)var_y / 10.0f, -1.5f + (float)var_z / 10.0f });
        proj = transpose(proj);
        modl = transpose(modl);

        memcpy(ubData + 0 * 16, &proj[0][0], sizeof(mat4));
        memcpy(ubData + 1 * 16, &view[0][0], sizeof(mat4));

        //Present
        uma_mb_0->SubmitData(ub1->GetResource(), 2 * sizeof(mat4), ubData);
        uma_mb_0->SubmitData(ub2->GetResource(), sizeof(mat4), (void*)&modl[0][0]);

        cmdBuffer->Present({ 0, 1 }, swapchain, draws, acquire, submit, windowResize);
        frameIndex = (frameIndex + 1) % swapchainCI.swapchainCount;
        frameCount++;

        __android_log_write(ANDROID_LOG_INFO, "MIRU_TEST", std::to_string(width).c_str());
        __android_log_write(ANDROID_LOG_INFO, "MIRU_TEST", std::to_string(height).c_str());
    }
    context->DeviceWaitIdle();
}

}
