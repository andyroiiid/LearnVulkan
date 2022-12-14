//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <memory>

#include "VulkanBase.h"
#include "VulkanPipeline.h"

class Renderer {
public:
    explicit Renderer(GLFWwindow *window);

    ~Renderer();

    Renderer(const Renderer &) = delete;

    Renderer &operator=(const Renderer &) = delete;

    Renderer(Renderer &&) = delete;

    Renderer &operator=(Renderer &&) = delete;

    void Frame(float deltaTime);

private:
    void CreateRenderPass();

    void CreateFramebuffers();

    void CreatePipeline();

    void CreateVertexBuffer();

    std::unique_ptr<VulkanBase> m_device;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;

    std::unique_ptr<VulkanPipeline> m_pipeline;

    VulkanBuffer m_vertexBuffer;

    float m_rotation = 0.0f;
};
