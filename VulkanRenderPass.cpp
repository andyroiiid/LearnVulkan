//
// Created by andyroiiid on 12/17/2022.
//

#include "VulkanRenderPass.h"

VulkanRenderPass::VulkanRenderPass(
        VulkanDevice *device,
        const std::initializer_list<VkFormat> &colorAttachmentFormats,
        VkFormat depthStencilAttachmentFormat,
        bool forPresent
) : m_device(device) {
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    VkAttachmentReference depthStencilAttachmentRef;

    int attachmentIndex = 0;

    for (VkFormat colorAttachmentFormat: colorAttachmentFormats) {
        VkAttachmentDescription &attachment = attachments.emplace_back();
        attachment.flags = 0;
        attachment.format = colorAttachmentFormat;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = forPresent ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference &attachmentReference = colorAttachmentRefs.emplace_back();
        attachmentReference.attachment = attachmentIndex++;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if (depthStencilAttachmentFormat != VK_FORMAT_UNDEFINED) {
        VkAttachmentDescription &attachment = attachments.emplace_back();
        attachment.flags = 0;
        attachment.format = depthStencilAttachmentFormat;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference &attachmentReference = depthStencilAttachmentRef;
        attachmentReference.attachment = attachmentIndex++;
        attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorAttachmentRefs.size();
    subpass.pColorAttachments = colorAttachmentRefs.data();

    if (depthStencilAttachmentFormat != VK_FORMAT_UNDEFINED) {
        subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;
    }

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    m_renderPass = m_device->CreateRenderPass(createInfo);
}

void VulkanRenderPass::Release() {
    if (m_device) {
        m_device->DestroyRenderPass(m_renderPass);
    }

    m_device = nullptr;
    m_renderPass = VK_NULL_HANDLE;
}

void VulkanRenderPass::Swap(VulkanRenderPass &other) noexcept {
    std::swap(m_device, other.m_device);
    std::swap(m_renderPass, other.m_renderPass);
}
