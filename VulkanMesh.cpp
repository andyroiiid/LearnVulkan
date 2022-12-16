//
// Created by andyroiiid on 12/15/2022.
//

#include "VulkanMesh.h"

VulkanMesh::VulkanMesh(VulkanBase *device, size_t vertexCount, size_t vertexSize, const void *data) {
    VkDeviceSize size = vertexCount * vertexSize;

    VulkanBuffer uploadBuffer = device->CreateBuffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST
    );
    uploadBuffer.Upload(size, data);

    VulkanBuffer vertexBuffer = device->CreateBuffer(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            0,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    );

    device->ImmediateSubmit([size, &uploadBuffer, &vertexBuffer](VkCommandBuffer cmd) {
        VkBufferCopy copy{};
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = size;
        vkCmdCopyBuffer(cmd, uploadBuffer.Get(), vertexBuffer.Get(), 1, &copy);
    });

    m_vertexBuffer = std::move(vertexBuffer);
    m_vertexCount = vertexCount;
}

void VulkanMesh::Release() {
    m_vertexBuffer = {};
    m_vertexCount = 0;
}

void VulkanMesh::Swap(VulkanMesh &other) noexcept {
    std::swap(m_vertexBuffer, other.m_vertexBuffer);
    std::swap(m_vertexCount, other.m_vertexCount);
}

void VulkanMesh::BindAndDraw(VkCommandBuffer commandBuffer) {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.Get(), &offset);
    vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
}
