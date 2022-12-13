//
// Created by andyroiiid on 12/12/2022.
//

#include "VertexBase.h"

#include <vector>
#include <vulkan/vulkan.h>

static inline VkPipelineVertexInputStateCreateInfo CreatePipelineVertexInputStateCreateInfo(
        const std::vector<VkVertexInputBindingDescription> &bindings,
        const std::vector<VkVertexInputAttributeDescription> &attributes
) {
    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = bindings.size();
    vertexInputState.pVertexBindingDescriptions = bindings.data();
    vertexInputState.vertexAttributeDescriptionCount = attributes.size();
    vertexInputState.pVertexAttributeDescriptions = attributes.data();
    return vertexInputState;
}

VkPipelineVertexInputStateCreateInfo VertexBase::GetPipelineVertexInputStateCreateInfo() {
    static const std::vector<VkVertexInputBindingDescription> bindings{
            {0, sizeof(VertexBase), VK_VERTEX_INPUT_RATE_VERTEX}
    };

    static const std::vector<VkVertexInputAttributeDescription> attributes{
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexBase, Position))},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexBase, Normal))},
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(VertexBase, Color))}
    };

    return CreatePipelineVertexInputStateCreateInfo(bindings, attributes);
}
