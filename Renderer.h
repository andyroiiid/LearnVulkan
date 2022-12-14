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

    void OnKeyDown(int key);

    void OnKeyUp(int key);

private:
    void CreateRenderPass();

    void CreateFramebuffers();

    void CreatePipeline();

    void CreateVertexBuffer();

    GLFWwindow *m_window = nullptr;
    std::unique_ptr<VulkanBase> m_device;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;

    bool m_fill = true;
    std::unique_ptr<VulkanPipeline> m_fillPipeline;
    std::unique_ptr<VulkanPipeline> m_wirePipeline;

    VulkanBuffer m_vertexBuffer;

    float m_rotationSpeed = 0.0f;
    float m_rotation = 0.0f;
};
