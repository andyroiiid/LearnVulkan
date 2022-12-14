//
// Created by andyroiiid on 12/13/2022.
//

#pragma once

#include "VulkanDevice.h"

class VulkanBase : public VulkanDevice {
public:
    explicit VulkanBase(GLFWwindow *window, bool vsync = true, size_t numBuffering = 2);

    ~VulkanBase();

    VulkanBase(const VulkanBase &) = delete;

    VulkanBase &operator=(const VulkanBase &) = delete;

    VulkanBase(VulkanBase &&) = delete;

    VulkanBase &operator=(VulkanBase &&) = delete;

    [[nodiscard]] const VkExtent2D &GetSwapchainExtent() const { return m_swapchainExtent; }

    [[nodiscard]] const std::vector<VkImageView> &GetSwapchainImageViews() const { return m_swapchainImageViews; }

    [[nodiscard]] const VkFormat &GetDepthStencilFormat() const { return m_depthStencilFormat; }

    [[nodiscard]] const std::vector<VkImageView> &GetDepthStencilImageViews() const { return m_depthStencilImageViews; }

    struct BeginFrameInfo {
        [[maybe_unused]] uint32_t SwapchainImageIndex;
        [[maybe_unused]] uint32_t BufferingIndex;
        [[maybe_unused]] VkCommandBuffer CommandBuffer;
    };

    BeginFrameInfo BeginFrame();

    void EndFrame();

private:
    void CreateSwapchain();

    void CreateSwapchainImageViews();

    void CreateDepthStencilImageAndViews();

    void CreateBufferingObjects();

    bool m_vsync = true;

    VkExtent2D m_swapchainExtent{};
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    VkFormat m_depthStencilFormat = VK_FORMAT_D32_SFLOAT;
    std::vector<VulkanImage> m_depthStencilImages;
    std::vector<VkImageView> m_depthStencilImageViews;

    struct BufferingObjects {
        VkFence RenderFence = VK_NULL_HANDLE;
        VkSemaphore PresentSemaphore = VK_NULL_HANDLE;
        VkSemaphore RenderSemaphore = VK_NULL_HANDLE;

        VkCommandPool CommandPool = VK_NULL_HANDLE;
        VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    };

    std::vector<BufferingObjects> m_bufferingObjects;

    uint32_t m_currentSwapchainImageIndex = 0;
    uint32_t m_currentFrameCount = 0;
    uint32_t m_currentBufferingIndex = 0;
};
