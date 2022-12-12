//
// Created by andyroiiid on 12/11/2022.
//

#pragma once

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

    void DestroyVulkan();

    GLFWwindow *m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    int m_graphicsQueueFamilyIndex = -1;
    int m_presentQueueFamilyIndex = -1;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
};
