//
// Created by andyroiiid on 12/17/2022.
//

#pragma once

#include "VulkanDevice.h"

class VulkanFramebuffer {
public:
    VulkanFramebuffer() = default;

    VulkanFramebuffer(
            VulkanDevice *device,
            VkRenderPass renderPass,
            const std::initializer_list<VkImageView> &attachments,
            uint32_t width,
            uint32_t height
    );

    ~VulkanFramebuffer() {
        Release();
    }

    VulkanFramebuffer(const VulkanFramebuffer &) = delete;

    VulkanFramebuffer &operator=(const VulkanFramebuffer &) = delete;

    VulkanFramebuffer(VulkanFramebuffer &&other) noexcept {
        Swap(other);
    }

    VulkanFramebuffer &operator=(VulkanFramebuffer &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanFramebuffer &other) noexcept;

    [[nodiscard]] const VkFramebuffer &Get() const { return m_framebuffer; }

private:
    VulkanDevice *m_device = nullptr;

    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
};
