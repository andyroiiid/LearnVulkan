//
// Created by andyroiiid on 12/12/2022.
//

#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "VertexBase.h"
#include "MeshUtilities.h"
#include "ImageFile.h"

struct CameraUniformData {
    glm::mat4 Projection;
    glm::mat4 View;
};

struct ModelConstantsData {
    glm::mat4 Model;
};

Renderer::Renderer(GLFWwindow *window) {
    m_window = window;
    m_device = std::make_unique<VulkanBase>(window, false);
    CreateRenderPass();
    CreateFramebuffers();
    CreateDescriptorSetLayout();
    CreateBufferingObjects();
    CreatePipeline();
    CreateMesh();
    CreateTexture();
    m_device->ImGuiInit(m_renderPass);
}

void Renderer::CreateRenderPass() {
    VkAttachmentDescription attachments[2];

    VkAttachmentDescription &colorAttachment = attachments[0];
    colorAttachment.flags = 0;
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

    VkAttachmentDescription &depthStencilAttachment = attachments[1];
    depthStencilAttachment.flags = 0;
    depthStencilAttachment.format = m_device->GetDepthStencilFormat();
    depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthStencilAttachmentRef{};
    depthStencilAttachmentRef.attachment = 1;
    depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 2;
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    m_renderPass = m_device->CreateRenderPass(createInfo);
}

void Renderer::CreateFramebuffers() {
    const std::vector<VkImageView> &swapchainImageViews = m_device->GetSwapchainImageViews();
    const std::vector<VkImageView> &depthStencilImageViews = m_device->GetDepthStencilImageViews();
    const VkExtent2D &swapchainExtent = m_device->GetSwapchainExtent();
    size_t numImages = swapchainImageViews.size();
    m_framebuffers.resize(numImages);
    for (int i = 0; i < numImages; i++) {
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = m_renderPass;
        VkImageView attachments[2]{
                swapchainImageViews[i],
                depthStencilImageViews[i]
        };
        createInfo.attachmentCount = 2;
        createInfo.pAttachments = attachments;
        createInfo.width = swapchainExtent.width;
        createInfo.height = swapchainExtent.height;
        createInfo.layers = 1;
        m_framebuffers[i] = m_device->CreateFramebuffer(createInfo);
    }
}

void Renderer::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding cameraUniformBinding{};
    cameraUniformBinding.binding = 0;
    cameraUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraUniformBinding.descriptorCount = 1;
    cameraUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &cameraUniformBinding;

    m_descriptorSetLayout = m_device->CreateDescriptorSetLayout(createInfo);
}

void Renderer::CreateBufferingObjects() {
    m_bufferingObjects.resize(m_device->GetNumBuffering());
    for (BufferingObjects &bufferingObjects: m_bufferingObjects) {
        VulkanBuffer cameraUniformBuffer = m_device->CreateBuffer(
                sizeof(CameraUniformData),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        );

        VkDescriptorSet descriptorSet = m_device->AllocateDescriptorSet(m_descriptorSetLayout);

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cameraUniformBuffer.Get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraUniformData);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;

        m_device->WriteDescriptorSet(writeDescriptorSet);

        bufferingObjects.CameraUniformBuffer = std::move(cameraUniformBuffer);
        bufferingObjects.DescriptorSet = descriptorSet;
    }
}

void Renderer::CreatePipeline() {
    VulkanPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.Device = m_device.get();
    pipelineCreateInfo.DescriptorSetLayout = m_descriptorSetLayout;
    pipelineCreateInfo.PushConstantSize = sizeof(ModelConstantsData);
    pipelineCreateInfo.ShaderStages = {
            {VK_SHADER_STAGE_VERTEX_BIT,   R"GLSL(
#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec4 vColor;

layout (set = 0, binding = 0) uniform CameraUniformData {
    mat4 uProjection;
    mat4 uView;
};

layout (push_constant) uniform ModelConstantsData
{
    mat4 uModel;
};

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1);
    vColor = vec4(aTexCoord, 0, 1);
}
)GLSL"},
            {VK_SHADER_STAGE_FRAGMENT_BIT, R"GLSL(
#version 450 core

layout (location = 0) in vec4 vColor;

layout (location = 0) out vec4 fColor;

void main()
{
    fColor = vColor;
}
)GLSL"}
    };
    pipelineCreateInfo.VertexInput = &VertexBase::GetPipelineVertexInputStateCreateInfo();
    pipelineCreateInfo.RenderPass = m_renderPass;

    m_fillPipeline = VulkanPipeline(pipelineCreateInfo);

    pipelineCreateInfo.PolygonMode = VK_POLYGON_MODE_LINE;
    pipelineCreateInfo.CullMode = VK_CULL_MODE_NONE;
    m_wirePipeline = VulkanPipeline(pipelineCreateInfo);
}

void Renderer::CreateMesh() {
    std::vector<VertexBase> vertices;
    AppendBoxVertices(vertices, {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});
    m_mesh = VulkanMesh(m_device.get(), vertices.size(), sizeof(VertexBase), vertices.data());
}

void Renderer::CreateTexture() {
    ImageFile imageFile("test.png");

    VkDeviceSize size = imageFile.GetDataSize();

    VulkanBuffer uploadBuffer = m_device->CreateBuffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST
    );
    uploadBuffer.Upload(size, imageFile.GetData());

    VulkanImage image = m_device->CreateImage2D(
            VK_FORMAT_R8G8B8A8_UNORM,
            VkExtent2D{imageFile.GetWidth(), imageFile.GetHeight()},
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            0,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    );

    VkExtent3D extent{imageFile.GetWidth(), imageFile.GetHeight(), 1};

    m_device->ImmediateSubmit([extent, &uploadBuffer, &image](VkCommandBuffer cmd) {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.image = image.Get();
        VkImageSubresourceRange &subresourceRange = imageMemoryBarrier.subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &imageMemoryBarrier
        );

        VkBufferImageCopy imageCopy{};
        imageCopy.bufferOffset = 0;
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferImageHeight = 0;
        VkImageSubresourceLayers &subresourceLayers = imageCopy.imageSubresource;
        subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceLayers.mipLevel = 0;
        subresourceLayers.baseArrayLayer = 0;
        subresourceLayers.layerCount = 1;
        imageCopy.imageExtent = extent;
        vkCmdCopyBufferToImage(cmd, uploadBuffer.Get(), image.Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &imageMemoryBarrier
        );
    });

    m_image = std::move(image);

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = m_image.Get();
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageSubresourceRange &depthImageViewSubresourceRange = imageViewCreateInfo.subresourceRange;
    depthImageViewSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    depthImageViewSubresourceRange.baseMipLevel = 0;
    depthImageViewSubresourceRange.levelCount = 1;
    depthImageViewSubresourceRange.baseArrayLayer = 0;
    depthImageViewSubresourceRange.layerCount = 1;
    m_imageView = m_device->CreateImageView(imageViewCreateInfo);
}

Renderer::~Renderer() {
    m_device->WaitIdle();

    m_device->ImGuiShutdown();

    m_device->DestroyImageView(m_imageView);
    m_image = {};

    m_mesh = {};

    m_fillPipeline = {};
    m_wirePipeline = {};

    for (BufferingObjects &bufferingObjects: m_bufferingObjects) {
        bufferingObjects.CameraUniformBuffer = {};
        m_device->FreeDescriptorSet(bufferingObjects.DescriptorSet);
    }

    m_device->DestroyDescriptorSetLayout(m_descriptorSetLayout);
    for (auto &framebuffer: m_framebuffers) {
        m_device->DestroyFramebuffer(framebuffer);
    }
    m_device->DestroyRenderPass(m_renderPass);

    m_device.reset();
}

void Renderer::Frame(float deltaTime) {
    m_fps = 1.0f / deltaTime;
    m_rotation += glm::radians(deltaTime * m_rotationSpeed);

    auto [swapchainImageIndex, bufferingIndex, cmd] = m_device->BeginFrame();

    BufferingObjects &bufferingObjects = m_bufferingObjects[bufferingIndex];
    const VkExtent2D &swapchainExtent = m_device->GetSwapchainExtent();
    const glm::mat4 projection = glm::perspective(
            glm::radians(60.0f),
            static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height),
            0.1f,
            100.0f
    );
    const glm::mat4 view = glm::lookAt(
            glm::vec3(3.0f, 4.0f, -5.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
    );
    CameraUniformData cameraUniformData{projection, view};
    bufferingObjects.CameraUniformBuffer.Upload(sizeof(CameraUniformData), &cameraUniformData);

    VkClearValue clearValues[2];
    VkClearColorValue &clearColor = clearValues[0].color;
    clearColor.float32[0] = 0.4f;
    clearColor.float32[1] = 0.8f;
    clearColor.float32[2] = 1.0f;
    clearColor.float32[3] = 1.0f;
    VkClearDepthStencilValue &clearDepthStencil = clearValues[1].depthStencil;
    clearDepthStencil.depth = 1.0f;
    clearDepthStencil.stencil = 0;
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass;
    renderPassBeginInfo.framebuffer = m_framebuffers[swapchainImageIndex];
    renderPassBeginInfo.renderArea = {{0, 0}, m_device->GetSwapchainExtent()};
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VulkanPipeline &pipeline = m_fill ? m_fillPipeline : m_wirePipeline;
    pipeline.Bind(cmd);
    pipeline.BindDescriptorSet(cmd, bufferingObjects.DescriptorSet);
    const glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    const ModelConstantsData constantsData{model};
    pipeline.PushConstants(cmd, constantsData);
    m_mesh.BindAndDraw(cmd);

    if (m_showImGui) {
        m_device->ImGuiNewFrame();
        if (ImGui::BeginMainMenuBar()) {
            ImGui::Text("fps = %f", m_fps);
        }
        ImGui::EndMainMenuBar();
        ImGui::Checkbox("Set Filled", &m_fill);
        m_device->ImGuiRender(cmd);
    }

    vkCmdEndRenderPass(cmd);

    m_device->EndFrame();
}

void Renderer::OnKeyDown(int key) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_TAB) {
        m_showImGui = !m_showImGui;
    }
    if (key == GLFW_KEY_SPACE) {
        m_rotationSpeed = 90.0f;
    }
}

void Renderer::OnKeyUp(int key) {
    if (key == GLFW_KEY_SPACE) {
        m_rotationSpeed = 0.0f;
    }
}
