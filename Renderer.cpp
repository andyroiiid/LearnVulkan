//
// Created by andyroiiid on 12/12/2022.
//

#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "VertexBase.h"
#include "MeshUtilities.h"
#include "ImageFile.h"

struct EngineUniformData {
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
    CreateDescriptorSetLayouts();
    CreateBufferingObjects();
    CreatePipeline();
    CreateMesh();
    CreateTexture();
    CreateMaterial();
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

void Renderer::CreateDescriptorSetLayouts() {
    VkDescriptorSetLayoutBinding engineUniformBinding{};
    engineUniformBinding.binding = 0;
    engineUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    engineUniformBinding.descriptorCount = 1;
    engineUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo engineDescriptorSetLayoutCreateInfo{};
    engineDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
    engineDescriptorSetLayoutCreateInfo.pBindings = &engineUniformBinding;

    m_engineDescriptorSetLayout = m_device->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);

    VkDescriptorSetLayoutBinding materialTextureBinding{};
    materialTextureBinding.binding = 0;
    materialTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    materialTextureBinding.descriptorCount = 1;
    materialTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo materialDescriptorSetLayoutCreateInfo{};
    materialDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    materialDescriptorSetLayoutCreateInfo.bindingCount = 1;
    materialDescriptorSetLayoutCreateInfo.pBindings = &materialTextureBinding;

    m_materialDescriptorSetLayout = m_device->CreateDescriptorSetLayout(materialDescriptorSetLayoutCreateInfo);
}

void Renderer::CreateBufferingObjects() {
    m_bufferingObjects.resize(m_device->GetNumBuffering());
    for (BufferingObjects &bufferingObjects: m_bufferingObjects) {
        VulkanBuffer engineUniformBuffer = m_device->CreateBuffer(
                sizeof(EngineUniformData),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        );

        VkDescriptorSet descriptorSet = m_device->AllocateDescriptorSet(m_engineDescriptorSetLayout);

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = engineUniformBuffer.Get();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(EngineUniformData);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;

        m_device->WriteDescriptorSet(writeDescriptorSet);

        bufferingObjects.EngineUniformBuffer = std::move(engineUniformBuffer);
        bufferingObjects.EngineDescriptorSet = descriptorSet;
    }
}

void Renderer::CreatePipeline() {
    VulkanPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.Device = m_device.get();
    pipelineCreateInfo.DescriptorSetLayouts = {
            m_engineDescriptorSetLayout,
            m_materialDescriptorSetLayout
    };
    pipelineCreateInfo.PushConstantSize = sizeof(ModelConstantsData);
    pipelineCreateInfo.ShaderStages = {
            {VK_SHADER_STAGE_VERTEX_BIT,   R"GLSL(
#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec3 vWorldNormal;
layout (location = 1) out vec2 vTexCoord;

layout (set = 0, binding = 0) uniform EngineUniformData {
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
    vWorldNormal = (uModel * vec4(aNormal, 1)).xyz;
    vTexCoord = aTexCoord;
}
)GLSL"},
            {VK_SHADER_STAGE_FRAGMENT_BIT, R"GLSL(
#version 450 core

layout (location = 0) in vec3 vWorldNormal;
layout (location = 1) in vec2 vTexCoord;

layout (location = 0) out vec4 fColor;

layout (set = 1, binding = 0) uniform sampler2D uTexture;

float LightDiffuse(vec3 worldNormal, vec3 lightDirection) {
    return max(0, dot(worldNormal, normalize(lightDirection)));
}

void main()
{
    vec3 worldNormal = normalize(vWorldNormal);
    vec4 color = texture(uTexture, vTexCoord);
    color.rgb *= LightDiffuse(worldNormal, vec3(1, 2, -3));
    fColor = color;
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
    m_texture = VulkanTexture(m_device.get(), imageFile.GetWidth(), imageFile.GetHeight(), imageFile.GetData());
}

void Renderer::CreateMaterial() {
    m_materialDescriptorSet = m_device->AllocateDescriptorSet(m_materialDescriptorSetLayout);
    m_texture.BindToDescriptorSet(m_materialDescriptorSet, 0);
}

Renderer::~Renderer() {
    m_device->WaitIdle();

    m_device->ImGuiShutdown();

    m_device->FreeDescriptorSet(m_materialDescriptorSet);

    m_texture = {};

    m_mesh = {};

    m_fillPipeline = {};
    m_wirePipeline = {};

    for (BufferingObjects &bufferingObjects: m_bufferingObjects) {
        bufferingObjects.EngineUniformBuffer = {};
        m_device->FreeDescriptorSet(bufferingObjects.EngineDescriptorSet);
    }

    m_device->DestroyDescriptorSetLayout(m_engineDescriptorSetLayout);
    m_device->DestroyDescriptorSetLayout(m_materialDescriptorSetLayout);
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
    EngineUniformData engineUniformData{projection, view};
    bufferingObjects.EngineUniformBuffer.Upload(sizeof(EngineUniformData), &engineUniformData);

    VkClearValue clearValues[2];
    clearValues[0].color = m_clearColor;
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
    pipeline.BindDescriptorSet(cmd, bufferingObjects.EngineDescriptorSet, 0);
    pipeline.BindDescriptorSet(cmd, m_materialDescriptorSet, 1);
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

        ImGui::ColorEdit4("Clear Color", m_clearColor.float32);
        ImGui::Checkbox("Cube Filled", &m_fill);
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
