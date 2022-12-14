//
// Created by andyroiiid on 12/14/2022.
//

#pragma once

#include <glm/mat4x4.hpp>

#include "VulkanBase.h"

struct VulkanPipelineCreateInfo {
    uint32_t PushConstantSize = 0;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    const VkPipelineVertexInputStateCreateInfo *VertexInput = nullptr;
};

class VulkanPipeline {
public:
    VulkanPipeline(VulkanBase *device, const VulkanPipelineCreateInfo &createInfo);

    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline &) = delete;

    VulkanPipeline &operator=(const VulkanPipeline &) = delete;

    VulkanPipeline(VulkanPipeline &&) = delete;

    VulkanPipeline &operator=(VulkanPipeline &&) = delete;

    void Bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    template<class T>
    void PushConstant(VkCommandBuffer commandBuffer, const T &pushConstantData) {
        vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(T), &pushConstantData);
    }

private:
    void CreatePipelineLayout(const VulkanPipelineCreateInfo &createInfo);

    void CreateShaders();

    void CreatePipeline(const VulkanPipelineCreateInfo &createInfo);

    VulkanBase *m_device = nullptr;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragmentShader = VK_NULL_HANDLE;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
};
