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

struct GpuImage {
    VkImage Image = VK_NULL_HANDLE;
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

    [[nodiscard]] const VkSurfaceFormatKHR &GetSurfaceFormat() const { return m_surfaceFormat; }

    [[nodiscard]] const VkExtent2D &GetSwapchainExtent() const { return m_swapchainExtent; }

    [[nodiscard]] const std::vector<VkImageView> &GetSwapchainImageViews() const { return m_swapchainImageViews; }

    [[nodiscard]] const VkFormat &GetDepthStencilFormat() const { return m_depthStencilFormat; }

    [[nodiscard]] const std::vector<VkImageView> &GetDepthStencilImageViews() const { return m_depthStencilImageViews; }

    VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo &createInfo);

    VkFramebuffer CreateFramebuffer(const VkFramebufferCreateInfo &createInfo);

    VkShaderModule CreateShaderModule(const VkShaderModuleCreateInfo &createInfo);

    VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo);

    VkPipeline CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo);

    GpuBuffer CreateBuffer(const VkBufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo);

    GpuBuffer CreateBuffer(
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

    GpuImage CreateImage(const VkImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo);

    GpuImage CreateImage2D(
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

    void DestroyBuffer(GpuBuffer buffer) { vmaDestroyBuffer(m_allocator, buffer.Buffer, buffer.Allocation); }

    void DestroyImage(GpuImage image) { vmaDestroyImage(m_allocator, image.Image, image.Allocation); }

    void DestroyImageView(VkImageView imageView) { vkDestroyImageView(m_device, imageView, nullptr); }

    VkResult WaitIdle() { return vkDeviceWaitIdle(m_device); }

    std::tuple<uint32_t, VkCommandBuffer> BeginFrame();

    void EndFrame();

    void *MapMemory(VmaAllocation allocation);

    void UnmapMemory(VmaAllocation allocation);

private:
    void CreateInstance();

    void CreateDebugMessenger();

    void CreateSurface();

    void SelectPhysicalDeviceAndQueueFamilyIndices();

    void CreateDevice();

    void CreateAllocator();

    void CreateSwapchain();

    void CreateSwapchainImageViews();

    void CreateDepthStencilImageAndViews();

    void CreateSyncPrimitives();

    void CreateCommandPoolAndBuffer();

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

    VkExtent2D m_swapchainExtent{};
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    VkFormat m_depthStencilFormat = VK_FORMAT_D32_SFLOAT;
    std::vector<GpuImage> m_depthStencilImages;
    std::vector<VkImageView> m_depthStencilImageViews;

    uint32_t m_currentSwapchainImageIndex = 0;

    VkFence m_renderFence = VK_NULL_HANDLE;
    VkSemaphore m_presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderSemaphore = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
};
