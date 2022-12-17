//
// Created by andyroiiid on 12/17/2022.
//

#include "VulkanTexture.h"

VulkanTexture::VulkanTexture(VulkanBase *device, uint32_t width, uint32_t height, const void *data)
        : m_device(device) {
    CreateImage(width, height, data);
    CreateImageView();
    CreateSampler();
}

void VulkanTexture::CreateImage(uint32_t width, uint32_t height, const void *data) {
    VkDeviceSize size = width * height * 4;

    VulkanBuffer uploadBuffer = m_device->CreateBuffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST
    );
    uploadBuffer.Upload(size, data);

    VulkanImage image = m_device->CreateImage2D(
            VK_FORMAT_R8G8B8A8_UNORM,
            VkExtent2D{width, height},
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            0,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    );

    VkExtent3D extent{width, height, 1};

    m_device->ImmediateSubmit([extent, &uploadBuffer, &image](VkCommandBuffer cmd) {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.image = image.Get();
        VkImageSubresourceRange &subresourceRange = imageMemoryBarrier.subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &imageMemoryBarrier
        );

        VkBufferImageCopy imageCopy{};
        imageCopy.bufferOffset = 0;
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferImageHeight = 0;
        VkImageSubresourceLayers &subresourceLayers = imageCopy.imageSubresource;
        subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceLayers.mipLevel = 0;
        subresourceLayers.baseArrayLayer = 0;
        subresourceLayers.layerCount = 1;
        imageCopy.imageExtent = extent;
        vkCmdCopyBufferToImage(cmd, uploadBuffer.Get(), image.Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &imageMemoryBarrier
        );
    });

    m_image = std::move(image);
}

void VulkanTexture::CreateImageView() {
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = m_image.Get();
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageSubresourceRange &depthImageViewSubresourceRange = imageViewCreateInfo.subresourceRange;
    depthImageViewSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    depthImageViewSubresourceRange.baseMipLevel = 0;
    depthImageViewSubresourceRange.levelCount = 1;
    depthImageViewSubresourceRange.baseArrayLayer = 0;
    depthImageViewSubresourceRange.layerCount = 1;
    m_imageView = m_device->CreateImageView(imageViewCreateInfo);
}

void VulkanTexture::CreateSampler() {
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    m_sampler = m_device->CreateSampler(samplerCreateInfo);
}

void VulkanTexture::Release() {
    if (m_device) {
        m_device->DestroySampler(m_sampler);
        m_device->DestroyImageView(m_imageView);
    }

    m_device = nullptr;
    m_image = {};
    m_imageView = VK_NULL_HANDLE;
    m_sampler = VK_NULL_HANDLE;
}

void VulkanTexture::Swap(VulkanTexture &other) noexcept {
    std::swap(m_device, other.m_device);
    std::swap(m_image, other.m_image);
    std::swap(m_imageView, other.m_imageView);
    std::swap(m_sampler, other.m_sampler);
}

void VulkanTexture::BindToDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_sampler;
    imageInfo.imageView = m_imageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &imageInfo;

    m_device->WriteDescriptorSet(writeDescriptorSet);
}
