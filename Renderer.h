//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include "GpuDevice.h"

class Renderer {
public:
    explicit Renderer(GpuDevice *device);

    ~Renderer();

    Renderer(const Renderer &) = delete;

    Renderer &operator=(const Renderer &) = delete;

    Renderer(Renderer &&) = delete;

    Renderer &operator=(Renderer &&) = delete;

    void Draw();

private:
    void CreateCommandBuffer();

    void CreateRenderPass();

    void CreateFramebuffers();

    GpuDevice *m_device = nullptr;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;
};
