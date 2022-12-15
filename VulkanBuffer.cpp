//
// Created by andyroiiid on 12/14/2022.
//

#include "VulkanBuffer.h"

#include "Debug.h"

VulkanBuffer::VulkanBuffer(
        VmaAllocator allocator,
        VkDeviceSize size,
        VkBufferUsageFlags bufferUsage,
        VmaAllocationCreateFlags flags,
        VmaMemoryUsage memoryUsage
) : m_allocator(allocator) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = bufferUsage;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;

    DebugCheckCriticalVk(
            vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocationCreateInfo, &m_buffer, &m_allocation, nullptr),
            "Failed to create Vulkan buffer."
    );
}

void VulkanBuffer::Release() {
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);

    m_allocator = VK_NULL_HANDLE;
    m_buffer = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
}

void VulkanBuffer::Swap(VulkanBuffer &other) noexcept {
    std::swap(m_allocator, other.m_allocator);
    std::swap(m_buffer, other.m_buffer);
    std::swap(m_allocation, other.m_allocation);
}

void VulkanBuffer::Upload(size_t size, const void *data) {
    void *mappedMemory = nullptr;
    DebugCheckCriticalVk(
            vmaMapMemory(m_allocator, m_allocation, &mappedMemory),
            "Failed to map Vulkan memory."
    );
    memcpy(mappedMemory, data, size);
    vmaUnmapMemory(m_allocator, m_allocation);
}
