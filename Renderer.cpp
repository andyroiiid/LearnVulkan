//
// Created by andyroiiid on 12/12/2022.
//

#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Debug.h"
#include "VertexBase.h"
#include "MeshUtilities.h"

struct PushConstantData {
    glm::mat4 Matrix;
};

Renderer::Renderer(GLFWwindow *window) {
    m_device = std::make_unique<VulkanBase>(window);

    CreateRenderPass();
    CreateFramebuffers();

    VulkanPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.PushConstantSize = sizeof(PushConstantData);
    pipelineCreateInfo.RenderPass = m_renderPass;
    pipelineCreateInfo.VertexInput = &VertexBase::GetPipelineVertexInputStateCreateInfo();
    m_pipeline = std::make_unique<VulkanPipeline>(m_device.get(), pipelineCreateInfo);

    CreateVertexBuffer();
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

void Renderer::CreateVertexBuffer() {
    std::vector<VertexBase> vertices;
    AppendBoxVertices(vertices, {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});

    m_vertexBuffer = m_device->CreateBuffer(
            vertices.size() * sizeof(VertexBase),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO
    );

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

    m_pipeline.reset();

    for (auto &framebuffer: m_framebuffers) {
        m_device->DestroyFramebuffer(framebuffer);
    }
    m_device->DestroyRenderPass(m_renderPass);

    m_device.reset();
}

void Renderer::Frame(float deltaTime) {
    m_rotation += glm::radians(deltaTime * 90.0f);

    auto [swapchainImageIndex, bufferingIndex, cmd] = m_device->BeginFrame();

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

    m_pipeline->Bind(cmd);
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
    const glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    const PushConstantData pushConstant{
            projection * view * model
    };
    m_pipeline->PushConstant(cmd, pushConstant);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer.Buffer, &offset);
    vkCmdDraw(cmd, 36, 1, 0, 0);

    vkCmdEndRenderPass(cmd);

    m_device->EndFrame();
}
