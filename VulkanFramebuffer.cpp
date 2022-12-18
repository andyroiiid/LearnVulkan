//
// Created by andyroiiid on 12/17/2022.
//

#include "VulkanFramebuffer.h"

VulkanFramebuffer::VulkanFramebuffer(
        VulkanDevice *device,
        VkRenderPass renderPass,
        const std::initializer_list<VkImageView> &attachments,
        uint32_t width,
        uint32_t height
) : m_device(device) {
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = data(attachments);
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;
    m_framebuffer = m_device->CreateFramebuffer(createInfo);
}

void VulkanFramebuffer::Release() {
    if (m_device) {
        m_device->DestroyFramebuffer(m_framebuffer);
    }

    m_device = nullptr;
    m_framebuffer = VK_NULL_HANDLE;
}

void VulkanFramebuffer::Swap(VulkanFramebuffer &other) noexcept {
    std::swap(m_device, other.m_device);
    std::swap(m_framebuffer, other.m_framebuffer);
}
