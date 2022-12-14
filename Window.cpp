//
// Created by andyroiiid on 12/11/2022.
//

#include "Window.h"

#include "Debug.h"

Window::Window() {
    DebugCheckCritical(glfwInit() == GLFW_TRUE, "Failed to init GLFW.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    m_window = glfwCreateWindow(800, 600, "Learn Vulkan", nullptr, nullptr);
    DebugCheckCritical(m_window != nullptr, "Failed to create GLFW window.");

    m_gpu = std::make_unique<VulkanDevice>(m_window);
    m_renderer = std::make_unique<Renderer>(m_gpu.get());
}

Window::~Window() {
    m_renderer.reset();
    m_gpu.reset();

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void Window::MainLoop() {
    glfwShowWindow(m_window);
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        m_renderer->Draw();
    }
}
