//
// Created by andyroiiid on 12/13/2022.
//

#pragma once

#include "VulkanDevice.h"

class VulkanBase : public VulkanDevice {
public:
    explicit VulkanBase(GLFWwindow *window);

    ~VulkanBase();

    VulkanBase(const VulkanBase &) = delete;

    VulkanBase &operator=(const VulkanBase &) = delete;

    VulkanBase(VulkanBase &&) = delete;

    VulkanBase &operator=(VulkanBase &&) = delete;

    [[nodiscard]] const VkExtent2D &GetSwapchainExtent() const { return m_swapchainExtent; }

    [[nodiscard]] const std::vector<VkImageView> &GetSwapchainImageViews() const { return m_swapchainImageViews; }

    [[nodiscard]] const VkFormat &GetDepthStencilFormat() const { return m_depthStencilFormat; }

    [[nodiscard]] const std::vector<VkImageView> &GetDepthStencilImageViews() const { return m_depthStencilImageViews; }

    std::tuple<uint32_t, VkCommandBuffer> BeginFrame();

    void EndFrame();

private:
    void CreateSwapchain();

    void CreateSwapchainImageViews();

    void CreateDepthStencilImageAndViews();

    void CreateSyncPrimitives();

    void CreateCommandPoolAndBuffer();

    VkExtent2D m_swapchainExtent{};
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    VkFormat m_depthStencilFormat = VK_FORMAT_D32_SFLOAT;
    std::vector<VulkanImage> m_depthStencilImages;
    std::vector<VkImageView> m_depthStencilImageViews;

    uint32_t m_currentSwapchainImageIndex = 0;

    VkFence m_renderFence = VK_NULL_HANDLE;
    VkSemaphore m_presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderSemaphore = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
};
