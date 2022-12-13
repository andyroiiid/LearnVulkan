//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <glm/vec3.hpp>

struct VkPipelineVertexInputStateCreateInfo;

struct VertexBase {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Color;

    static VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo();
};
