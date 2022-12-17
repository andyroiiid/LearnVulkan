//
// Created by andyroiiid on 12/17/2022.
//

#pragma once

#include "VulkanBase.h"
#include "VulkanImage.h"

class VulkanTexture {
public:
    VulkanTexture() = default;

    VulkanTexture(VulkanBase *device, uint32_t width, uint32_t height, const void *data);

    ~VulkanTexture() {
        Release();
    }

    VulkanTexture(const VulkanTexture &) = delete;

    VulkanTexture &operator=(const VulkanTexture &) = delete;

    VulkanTexture(VulkanTexture &&other) noexcept {
        Swap(other);
    }

    VulkanTexture &operator=(VulkanTexture &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanTexture &other) noexcept;

    void BindToDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding);

private:
    void CreateImage(uint32_t width, uint32_t height, const void *data);

    void CreateImageView();

    void CreateSampler();

    VulkanBase *m_device = nullptr;

    VulkanImage m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};
