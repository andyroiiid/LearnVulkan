//
// Created by andyroiiid on 12/12/2022.
//

#pragma once

#include <vector>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

struct GpuBuffer {
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;
};

class GpuDevice {
public:
    explicit GpuDevice(GLFWwindow *window);

    ~GpuDevice();

    GpuDevice(const GpuDevice &) = delete;

    GpuDevice &operator=(const GpuDevice &) = delete;

    GpuDevice(GpuDevice &&) = delete;

    GpuDevice &operator=(GpuDevice &&) = delete;

    [[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }

    [[nodiscard]] const VkSurfaceFormatKHR &GetSurfaceFormat() const { return m_surfaceFormat; }

    [[nodiscard]] const VkExtent2D &GetSwapchainExtent() const { return m_swapchainExtent; }

    [[nodiscard]] const std::vector<VkImageView> &GetSwapchainImageViews() const { return m_swapchainImageViews; }

    VkCommandPool CreateCommandPool(const VkCommandPoolCreateInfo &createInfo);

    VkCommandBuffer AllocateCommandBuffer(const VkCommandBufferAllocateInfo &allocateInfo);

    VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo &createInfo);

    VkFramebuffer CreateFramebuffer(const VkFramebufferCreateInfo &createInfo);

    VkShaderModule CreateShaderModule(const VkShaderModuleCreateInfo &createInfo);

    VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo);

    VkPipeline CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo);

    GpuBuffer CreateBuffer(const VkBufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo);

    void DestroyCommandPool(VkCommandPool commandPool) { vkDestroyCommandPool(m_device, commandPool, nullptr); }

    void DestroyRenderPass(VkRenderPass renderPass) { vkDestroyRenderPass(m_device, renderPass, nullptr); }

    void DestroyFramebuffer(VkFramebuffer framebuffer) { vkDestroyFramebuffer(m_device, framebuffer, nullptr); }

    void DestroyShaderModule(VkShaderModule shaderModule) { vkDestroyShaderModule(m_device, shaderModule, nullptr); }

    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) { vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr); }

    void DestroyPipeline(VkPipeline pipeline) { vkDestroyPipeline(m_device, pipeline, nullptr); }

    void DestroyBuffer(GpuBuffer buffer) { vmaDestroyBuffer(m_allocator, buffer.Buffer, buffer.Allocation); }

    VkResult WaitIdle() { return vkDeviceWaitIdle(m_device); }

    uint32_t WaitForFrame();

    void SubmitAndPresent(uint32_t swapchainImageIndex, VkCommandBuffer commandBuffer);

    void *MapMemory(VmaAllocation allocation);

    void UnmapMemory(VmaAllocation allocation);

private:
    void CreateInstance();

    void CreateDebugMessenger();

    void CreateSurface();

    void SelectPhysicalDeviceAndQueueFamilyIndices();

    void CreateDevice();

    void CreateAllocator();

    void CreateSyncPrimitives();

    void CreateSwapchain();

    void CreateSwapchainImageViews();

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

    VkFence m_renderFence = VK_NULL_HANDLE;
    VkSemaphore m_presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderSemaphore = VK_NULL_HANDLE;

    VkExtent2D m_swapchainExtent{};
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    VmaAllocator m_allocator = VK_NULL_HANDLE;
};
