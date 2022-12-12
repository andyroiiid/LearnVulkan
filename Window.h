//
// Created by andyroiiid on 12/11/2022.
//

#pragma once

#include <vector>
#include <GLFW/glfw3.h>

class Window {
public:
    Window();

    ~Window();

    Window(const Window &) = delete;

    Window &operator=(const Window &) = delete;

    Window(Window &&) = delete;

    Window &operator=(Window &&) = delete;

    void MainLoop();

private:
    void InitVulkan();

    void CreateInstance();

    void CreateDebugMessenger();

    void CreateSurface();

    void SelectPhysicalDeviceAndGraphicsQueueFamilyIndex();

    void CreateDevice();

    void CreateSwapchain();

    void CreateSwapchainImageViews();

    void DestroyVulkan();

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

    VkExtent2D m_swapchainExtent{};
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
};
