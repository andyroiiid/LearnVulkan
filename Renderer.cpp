//
// Created by andyroiiid on 12/12/2022.
//

#include "Renderer.h"

#include "Debug.h"
#include "ShaderCompiler.h"
#include "VertexBase.h"

Renderer::Renderer(GpuDevice *device) {
    m_device = device;
    CreateCommandBuffer();
    CreateRenderPass();
    CreateFramebuffers();
    CreatePipelineLayout();
    CreateShaders();
    CreatePipeline();
    CreateVertexBuffer();
}

void Renderer::CreateCommandBuffer() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = m_device->GetGraphicsQueueFamilyIndex();
    m_commandPool = m_device->CreateCommandPool(createInfo);

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    m_commandBuffer = m_device->AllocateCommandBuffer(allocateInfo);
}

void Renderer::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_device->GetSurfaceFormat().format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &colorAttachment;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    m_renderPass = m_device->CreateRenderPass(createInfo);
}

void Renderer::CreateFramebuffers() {
    const std::vector<VkImageView> &swapchainImageViews = m_device->GetSwapchainImageViews();
    const VkExtent2D &swapchainExtent = m_device->GetSwapchainExtent();
    size_t numImages = swapchainImageViews.size();
    m_framebuffers.resize(numImages);
    for (int i = 0; i < numImages; i++) {
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = m_renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &swapchainImageViews[i];
        createInfo.width = swapchainExtent.width;
        createInfo.height = swapchainExtent.height;
        createInfo.layers = 1;
        m_framebuffers[i] = m_device->CreateFramebuffer(createInfo);
    }
}

void Renderer::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;
    m_pipelineLayout = m_device->CreatePipelineLayout(createInfo);
}

void Renderer::CreateShaders() {
    ShaderCompiler shaderCompiler;

    const char *vertexSource = R"GLSL(
#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

layout (location = 0) out vec4 vColor;

void main()
{
    gl_Position = vec4(aPosition, 1);
    vColor = vec4(aColor, 1);
}
)GLSL";
    const char *fragmentSource = R"GLSL(
#version 450 core

layout (location = 0) in vec4 vColor;

layout (location = 0) out vec4 fColor;

void main()
{
    fColor = vColor;
}
)GLSL";

    std::vector<uint32_t> vertexSpirv;
    std::vector<uint32_t> fragmentSpirv;

    DebugCheckCritical(
            shaderCompiler.Compile(EShLangVertex, vertexSource, vertexSpirv),
            "Failed to compile vertex shader."
    );
    DebugCheckCritical(
            shaderCompiler.Compile(EShLangFragment, fragmentSource, fragmentSpirv),
            "Failed to compile fragment shader."
    );

    VkShaderModuleCreateInfo vertexShaderCreateInfo{};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderCreateInfo.codeSize = vertexSpirv.size() * sizeof(uint32_t);
    vertexShaderCreateInfo.pCode = vertexSpirv.data();
    m_vertexShader = m_device->CreateShaderModule(vertexShaderCreateInfo);

    VkShaderModuleCreateInfo fragmentShaderCreateInfo{};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderCreateInfo.codeSize = fragmentSpirv.size() * sizeof(uint32_t);
    fragmentShaderCreateInfo.pCode = fragmentSpirv.data();
    m_fragmentShader = m_device->CreateShaderModule(fragmentShaderCreateInfo);
}

void Renderer::CreatePipeline() {
    std::vector<std::tuple<VkShaderStageFlagBits, VkShaderModule>> shaderStages = {
            {VK_SHADER_STAGE_VERTEX_BIT,   m_vertexShader},
            {VK_SHADER_STAGE_FRAGMENT_BIT, m_fragmentShader}
    };

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for (const auto &[stage, module]: shaderStages) {
        VkPipelineShaderStageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage = stage;
        createInfo.module = module;
        createInfo.pName = "main";
        stages.push_back(createInfo);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = VertexBase::GetPipelineVertexInputStateCreateInfo();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    const VkExtent2D &size = m_device->GetSwapchainExtent();
    const VkViewport viewport{
            0.0f, 0.0f,
            static_cast<float>(size.width), static_cast<float>(size.height),
            0.0f, 1.0f
    };
    const VkRect2D scissor{
            {0, 0},
            size
    };
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachmentState;

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = stages.size();
    createInfo.pStages = stages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.layout = m_pipelineLayout;
    createInfo.renderPass = m_renderPass;
    createInfo.subpass = 0;
    m_pipeline = m_device->CreateGraphicsPipeline(createInfo);
}

void Renderer::CreateVertexBuffer() {
    std::vector<VertexBase> vertices{
            {{-1.0f, 1.0f,  0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f,  1.0f,  0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.0f,  -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
    };

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = vertices.size() * sizeof(VertexBase);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    m_vertexBuffer = m_device->CreateBuffer(bufferCreateInfo, allocationCreateInfo);

    auto data = static_cast<VertexBase *>(m_device->MapMemory(m_vertexBuffer.Allocation));
    memcpy(data, vertices.data(), vertices.size() * sizeof(VertexBase));
    m_device->UnmapMemory(m_vertexBuffer.Allocation);
}

Renderer::~Renderer() {
    DebugCheckCriticalVk(
            m_device->WaitIdle(),
            "Failed to wait for Vulkan device when trying to cleanup renderer."
    );

    m_device->DestroyBuffer(m_vertexBuffer);
    m_device->DestroyPipeline(m_pipeline);
    m_device->DestroyShaderModule(m_vertexShader);
    m_device->DestroyShaderModule(m_fragmentShader);
    m_device->DestroyPipelineLayout(m_pipelineLayout);
    for (auto &framebuffer: m_framebuffers) {
        m_device->DestroyFramebuffer(framebuffer);
    }
    m_device->DestroyRenderPass(m_renderPass);
    m_device->DestroyCommandPool(m_commandPool);
}

void Renderer::Draw() {
    uint32_t swapchainImageIndex = m_device->WaitForFrame();

    DebugCheckCriticalVk(
            vkResetCommandBuffer(m_commandBuffer, 0),
            "Failed to reset Vulkan command buffer."
    );

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    DebugCheckCriticalVk(
            vkBeginCommandBuffer(m_commandBuffer, &commandBufferBeginInfo),
            "Failed to begin Vulkan command buffer."
    );

    VkClearValue clearValue{};
    clearValue.color.float32[0] = 0.4f;
    clearValue.color.float32[1] = 0.8f;
    clearValue.color.float32[2] = 1.0f;
    clearValue.color.float32[3] = 1.0f;
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass;
    renderPassBeginInfo.framebuffer = m_framebuffers[swapchainImageIndex];
    renderPassBeginInfo.renderArea = {{0, 0}, m_device->GetSwapchainExtent()};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &m_vertexBuffer.Buffer, &offset);
    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(m_commandBuffer);

    DebugCheckCriticalVk(
            vkEndCommandBuffer(m_commandBuffer),
            "Failed to end Vulkan command buffer."
    );

    m_device->SubmitAndPresent(swapchainImageIndex, m_commandBuffer);
}
