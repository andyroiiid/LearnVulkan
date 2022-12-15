//
// Created by andyroiiid on 12/14/2022.
//

#include "VulkanImage.h"

#include "Debug.h"

VulkanImage::VulkanImage(
        VmaAllocator allocator,
        VkFormat format,
        const VkExtent2D &extent,
        VkImageUsageFlags imageUsage,
        VmaAllocationCreateFlags flags,
        VmaMemoryUsage memoryUsage
) : m_allocator(allocator) {
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = {extent.width, extent.height, 1};
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.usage = imageUsage;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.flags = flags;
    allocationCreateInfo.usage = memoryUsage;

    DebugCheckCriticalVk(
            vmaCreateImage(m_allocator, &imageCreateInfo, &allocationCreateInfo, &m_image, &m_allocation, nullptr),
            "Failed to create Vulkan image."
    );
}

void VulkanImage::Release() {
    vmaDestroyImage(m_allocator, m_image, m_allocation);

    m_allocator = VK_NULL_HANDLE;
    m_image = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
}

void VulkanImage::Swap(VulkanImage &other) noexcept {
    std::swap(m_allocator, other.m_allocator);
    std::swap(m_image, other.m_image);
    std::swap(m_allocation, other.m_allocation);
}
