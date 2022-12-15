//
// Created by andyroiiid on 12/14/2022.
//

#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class VulkanBuffer {
public:
    VulkanBuffer() = default;

    VulkanBuffer(
            VmaAllocator allocator,
            VkDeviceSize size,
            VkBufferUsageFlags bufferUsage,
            VmaAllocationCreateFlags flags,
            VmaMemoryUsage memoryUsage
    );

    ~VulkanBuffer() {
        Release();
    }

    VulkanBuffer(const VulkanBuffer &) = delete;

    VulkanBuffer &operator=(const VulkanBuffer &) = delete;

    VulkanBuffer(VulkanBuffer &&other) noexcept {
        Swap(other);
    }

    VulkanBuffer &operator=(VulkanBuffer &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanBuffer &other) noexcept;

    void Upload(size_t size, const void *data);

    template<class T>
    void Upload(const std::vector<T> &data) {
        Upload(data.size() * sizeof(T), data.data());
    }

    template<class T>
    void Upload(const T &data) {
        Upload(sizeof(T), &data);
    }

    void BindAsVertexBuffer(VkCommandBuffer commandBuffer);

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
};
