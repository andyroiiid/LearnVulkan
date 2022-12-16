//
// Created by andyroiiid on 12/15/2022.
//

#pragma once

#include "VulkanBase.h"
#include "VulkanBuffer.h"

class VulkanMesh {
public:
    VulkanMesh() = default;

    VulkanMesh(VulkanBase *device, size_t vertexCount, size_t vertexSize, const void *data);

    ~VulkanMesh() {
        Release();
    }

    VulkanMesh(const VulkanMesh &) = delete;

    VulkanMesh &operator=(const VulkanMesh &) = delete;

    VulkanMesh(VulkanMesh &&other) noexcept {
        Swap(other);
    }

    VulkanMesh &operator=(VulkanMesh &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanMesh &other) noexcept;

    void BindAndDraw(VkCommandBuffer commandBuffer);

private:
    VulkanBuffer m_vertexBuffer;
    uint32_t m_vertexCount = 0;
};
