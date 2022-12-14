//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

struct VulkanBuffer {
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;
};

struct VulkanImage {
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;
};

class VulkanDevice {
public:
    explicit VulkanDevice(GLFWwindow *window);

    ~VulkanDevice();

    VulkanDevice(const VulkanDevice &) = delete;

    VulkanDevice &operator=(const VulkanDevice &) = delete;

    VulkanDevice(VulkanDevice &&) = delete;

    VulkanDevice &operator=(VulkanDevice &&) = delete;

    [[nodiscard]] const VkSurfaceFormatKHR &GetSurfaceFormat() const { return m_surfaceFormat; }

    VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo &createInfo);

    VkFramebuffer CreateFramebuffer(const VkFramebufferCreateInfo &createInfo);

    VkShaderModule CreateShaderModule(const VkShaderModuleCreateInfo &createInfo);

    VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo);

    VkPipeline CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo);

    VulkanBuffer CreateBuffer(const VkBufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo);

    VulkanBuffer CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags bufferUsage,
            VmaAllocationCreateFlags flags,
            VmaMemoryUsage memoryUsage
    ) {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = bufferUsage;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.flags = flags;
        allocationCreateInfo.usage = memoryUsage;

        return CreateBuffer(bufferCreateInfo, allocationCreateInfo);
    }

    VulkanImage CreateImage(const VkImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo);

    VulkanImage CreateImage2D(
            VkFormat format,
            const VkExtent2D &extent,
            VkImageUsageFlags imageUsage,
            VmaAllocationCreateFlags flags,
            VmaMemoryUsage memoryUsage
    ) {
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

        return CreateImage(imageCreateInfo, allocationCreateInfo);
    }

    VkImageView CreateImageView(const VkImageViewCreateInfo &createInfo);

    void DestroyRenderPass(VkRenderPass renderPass) { vkDestroyRenderPass(m_device, renderPass, nullptr); }

    void DestroyFramebuffer(VkFramebuffer framebuffer) { vkDestroyFramebuffer(m_device, framebuffer, nullptr); }

    void DestroyShaderModule(VkShaderModule shaderModule) { vkDestroyShaderModule(m_device, shaderModule, nullptr); }

    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) { vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr); }

    void DestroyPipeline(VkPipeline pipeline) { vkDestroyPipeline(m_device, pipeline, nullptr); }

    void DestroyBuffer(VulkanBuffer buffer) { vmaDestroyBuffer(m_allocator, buffer.Buffer, buffer.Allocation); }

    void DestroyImage(VulkanImage image) { vmaDestroyImage(m_allocator, image.Image, image.Allocation); }

    void DestroyImageView(VkImageView imageView) { vkDestroyImageView(m_device, imageView, nullptr); }

    VkResult WaitIdle() { return vkDeviceWaitIdle(m_device); }

    void *MapMemory(VmaAllocation allocation);

    void UnmapMemory(VmaAllocation allocation);

protected:
    void CreateInstance();

    void CreateDebugMessenger();

    void CreateSurface();

    void SelectPhysicalDeviceAndQueueFamilyIndices();

    void CreateDevice();

    void CreateAllocator();

    GLFWwindow *m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamilyIndex = 0;
    uint32_t m_presentQueueFamilyIndex = 0;
    VkSurfaceFormatKHR m_surfaceFormat{};
    VkPresentModeKHR m_presentMode{};
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VmaAllocator m_allocator = VK_NULL_HANDLE;
};
