//
// Created by andyroiiid on 12/11/2022.
//

#pragma once

#include <memory>
#include <GLFW/glfw3.h>

#include "VulkanDevice.h"
#include "Renderer.h"

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
    GLFWwindow *m_window = nullptr;

    std::unique_ptr<VulkanDevice> m_gpu;
    std::unique_ptr<Renderer> m_renderer;
};
