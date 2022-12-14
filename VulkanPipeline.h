//
// Created by andyroiiid on 12/14/2022.
//

#pragma once

#include "VulkanBase.h"

struct VulkanShaderStageCreateInfo {
    VkShaderStageFlagBits Stage{};
    const char *Source = nullptr;
};

struct VulkanPipelineCreateInfo {
    VulkanBase *Device = nullptr;

    uint32_t PushConstantSize = 0;

    std::vector<VulkanShaderStageCreateInfo> ShaderStages;

    VkRenderPass RenderPass = VK_NULL_HANDLE;
    const VkPipelineVertexInputStateCreateInfo *VertexInput = nullptr;
};

class VulkanPipeline {
public:
    explicit VulkanPipeline(const VulkanPipelineCreateInfo &createInfo);

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
        vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(T), &pushConstantData);
    }

private:
    void CreatePipelineLayout(const VulkanPipelineCreateInfo &createInfo);

    void CreateShaderStages(const VulkanPipelineCreateInfo &createInfo);

    void CreatePipeline(const VulkanPipelineCreateInfo &createInfo);

    VulkanBase *m_device = nullptr;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

    struct ShaderStage {
        VkShaderStageFlagBits Stage{};
        VkShaderModule Module = VK_NULL_HANDLE;
    };
    std::vector<ShaderStage> m_shaderStages;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
};
