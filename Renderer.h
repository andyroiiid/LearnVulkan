//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include "VulkanDevice.h"

class Renderer {
public:
    explicit Renderer(VulkanDevice *device);

    ~Renderer();

    Renderer(const Renderer &) = delete;

    Renderer &operator=(const Renderer &) = delete;

    Renderer(Renderer &&) = delete;

    Renderer &operator=(Renderer &&) = delete;

    void Draw();

private:
    void CreateRenderPass();

    void CreateFramebuffers();

    void CreatePipelineLayout();

    void CreateShaders();

    void CreatePipeline();

    void CreateVertexBuffer();

    VulkanDevice *m_device = nullptr;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragmentShader = VK_NULL_HANDLE;

    VkPipeline m_pipeline = VK_NULL_HANDLE;

    VulkanBuffer m_vertexBuffer;
};
