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

    VkFence CreateFence(VkFenceCreateFlags flags = 0);

    void DestroyFence(VkFence fence) {
        vkDestroyFence(m_device, fence, nullptr);
    }

    void WaitForFence(VkFence fence, uint64_t timeout = 1'000'000'000);

    void ResetFence(VkFence fence);

    VkSemaphore CreateSemaphore();

    void DestroySemaphore(VkSemaphore semaphore) {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    VkCommandPool CreateCommandPool(VkCommandPoolCreateFlags flags = 0);

    void DestroyCommandPool(VkCommandPool commandPool) {
        vkDestroyCommandPool(m_device, commandPool, nullptr);
    }

    VkCommandBuffer AllocateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level);

    void FreeCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer) {
        vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
    }

    VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo &createInfo);

    void DestroyRenderPass(VkRenderPass renderPass) {
        vkDestroyRenderPass(m_device, renderPass, nullptr);
    }

    VkFramebuffer CreateFramebuffer(const VkFramebufferCreateInfo &createInfo);

    void DestroyFramebuffer(VkFramebuffer framebuffer) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    VkShaderModule CreateShaderModule(const VkShaderModuleCreateInfo &createInfo);

    void DestroyShaderModule(VkShaderModule shaderModule) {
        vkDestroyShaderModule(m_device, shaderModule, nullptr);
    }

    VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo);

    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) {
        vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
    }

    VkPipeline CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo);

    void DestroyPipeline(VkPipeline pipeline) {
        vkDestroyPipeline(m_device, pipeline, nullptr);
    }

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

    void DestroyImageView(VkImageView imageView) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo &createInfo);

    void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
    }

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout);

    void FreeDescriptorSet(VkDescriptorSet descriptorSet);

    void WriteDescriptorSet(const VkWriteDescriptorSet &writeDescriptorSet) {
        vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void WaitIdle();

    void WaitGraphicsQueueIdle();

    void SubmitToGraphicsQueue(const VkSubmitInfo &submitInfo, VkFence fence);

    void SubmitToGraphicsQueue(VkCommandBuffer commandBuffer, VkFence fence);

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

void BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags = 0);

void EndCommandBuffer(VkCommandBuffer commandBuffer);

void ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags = 0);
