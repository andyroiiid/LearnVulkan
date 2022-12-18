//
// Created by andyroiiid on 12/17/2022.
//

#pragma once

#include "VulkanDevice.h"

class VulkanRenderPass {
public:
    VulkanRenderPass() = default;

    VulkanRenderPass(
            VulkanDevice *device,
            const std::initializer_list<VkFormat> &colorAttachments,
            VkFormat depthStencilAttachmentFormat = VK_FORMAT_UNDEFINED,
            bool forPresent = false
    );

    ~VulkanRenderPass() {
        Release();
    }

    VulkanRenderPass(const VulkanRenderPass &) = delete;

    VulkanRenderPass &operator=(const VulkanRenderPass &) = delete;

    VulkanRenderPass(VulkanRenderPass &&other) noexcept {
        Swap(other);
    }

    VulkanRenderPass &operator=(VulkanRenderPass &&other) noexcept {
        if (this != &other) {
            Release();
            Swap(other);
        }
        return *this;
    }

    void Release();

    void Swap(VulkanRenderPass &other) noexcept;

    [[nodiscard]] const VkRenderPass &Get() const { return m_renderPass; }

private:
    VulkanDevice *m_device = nullptr;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};
