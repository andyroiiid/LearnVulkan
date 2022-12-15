//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

#include "VulkanBuffer.h"
#include "VulkanImage.h"

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

    VulkanBuffer CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags bufferUsage,
            VmaAllocationCreateFlags flags,
            VmaMemoryUsage memoryUsage
    ) {
        return {m_allocator, size, bufferUsage, flags, memoryUsage};
    }

    VulkanImage CreateImage2D(
            VkFormat format,
            const VkExtent2D &extent,
            VkImageUsageFlags imageUsage,
            VmaAllocationCreateFlags flags,
            VmaMemoryUsage memoryUsage
    ) {
        return {m_allocator, format, extent, imageUsage, flags, memoryUsage};
    }

    VkImageView CreateImageView(const VkImageViewCreateInfo &createInfo);

    void DestroyRenderPass(VkRenderPass renderPass) { vkDestroyRenderPass(m_device, renderPass, nullptr); }

    void DestroyFramebuffer(VkFramebuffer framebuffer) { vkDestroyFramebuffer(m_device, framebuffer, nullptr); }

    void DestroyShaderModule(VkShaderModule shaderModule) { vkDestroyShaderModule(m_device, shaderModule, nullptr); }

    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) { vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr); }

    void DestroyPipeline(VkPipeline pipeline) { vkDestroyPipeline(m_device, pipeline, nullptr); }

    void DestroyImageView(VkImageView imageView) { vkDestroyImageView(m_device, imageView, nullptr); }

    VkResult WaitIdle() { return vkDeviceWaitIdle(m_device); }

protected:
    void CreateInstance();

    void CreateDebugMessenger();

    void CreateSurface();

    void SelectPhysicalDeviceAndQueueFamilyIndices();

    void CreateDevice();

    void CreateAllocator();

    void CreateDescriptorPool();

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

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
