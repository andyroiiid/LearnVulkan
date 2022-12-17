//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <memory>

#include "VulkanBase.h"
#include "VulkanPipeline.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"

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

    void CreateDescriptorSetLayouts();

    void CreateBufferingObjects();

    void CreatePipeline();

    void CreateMesh();

    void CreateTexture();

    void CreateMaterial();

    GLFWwindow *m_window = nullptr;
    std::unique_ptr<VulkanBase> m_device;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;

    VkDescriptorSetLayout m_engineDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_materialDescriptorSetLayout = VK_NULL_HANDLE;

    struct BufferingObjects {
        VulkanBuffer EngineUniformBuffer;
        VkDescriptorSet EngineDescriptorSet;
    };
    std::vector<BufferingObjects> m_bufferingObjects;

    bool m_fill = true;
    VulkanPipeline m_fillPipeline;
    VulkanPipeline m_wirePipeline;

    VulkanMesh m_mesh;

    VulkanTexture m_texture;

    VkDescriptorSet m_materialDescriptorSet = VK_NULL_HANDLE;

    bool m_showImGui = true;

    float m_fps = 0.0f;

    VkClearColorValue m_clearColor{};

    float m_rotationSpeed = 0.0f;
    float m_rotation = 0.0f;
};
