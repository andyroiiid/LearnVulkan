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
    CreateDescriptorSetLayouts();
    CreateBufferingObjects();
    CreatePipeline();
    CreateMesh();
    CreateTexture();
    CreateMaterial();
    m_device->ImGuiInit();
}

void Renderer::CreateDescriptorSetLayouts() {
    m_engineDescriptorSetLayout = VulkanDescriptorSetLayout(
            m_device.get(),
            {
                    {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT}
            }
    );

    m_materialDescriptorSetLayout = VulkanDescriptorSetLayout(
            m_device.get(),
            {
                    {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
            }
    );
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

        VkDescriptorSet descriptorSet = m_engineDescriptorSetLayout.AllocateDescriptorSet();

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
            m_engineDescriptorSetLayout.Get(),
            m_materialDescriptorSetLayout.Get()
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
    pipelineCreateInfo.RenderPass = m_device->GetPrimaryRenderPass();

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
    m_materialDescriptorSet = m_materialDescriptorSetLayout.AllocateDescriptorSet();
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

    m_engineDescriptorSetLayout = {};
    m_materialDescriptorSetLayout = {};

    m_device.reset();
}

void Renderer::Frame(float deltaTime) {
    m_fps = 1.0f / deltaTime;
    m_rotation += glm::radians(deltaTime * m_rotationSpeed);

    auto [screenFramebuffer, bufferingIndex, cmd] = m_device->BeginFrame();

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
    renderPassBeginInfo.renderPass = m_device->GetPrimaryRenderPass();
    renderPassBeginInfo.framebuffer = screenFramebuffer;
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
