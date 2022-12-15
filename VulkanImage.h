//
// Created by andyroiiid on 12/14/2022.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class VulkanImage {
public:
    VulkanImage() = default;

    VulkanImage(
            VmaAllocator allocator,
            VkFormat format,
            const VkExtent2D &extent,
            VkImageUsageFlags imageUsage,
            VmaAllocationCreateFlags flags,
            VmaMemoryUsage memoryUsage
    );

    ~VulkanImage() {
        Release();
    }

    VulkanImage(const VulkanImage &) = delete;

    VulkanImage &operator=(const VulkanImage &) = delete;

    VulkanImage(VulkanImage &&other) noexcept {
        Swap(other);
    }

    VulkanImage &operator=(VulkanImage &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanImage &other) noexcept;

    [[nodiscard]] const VkImage &Get() const { return m_image; }

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkImage m_image = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
};
