//
// Created by andyroiiid on 12/11/2022.
//

#pragma once

#include <memory>
#include <GLFW/glfw3.h>

#include "GpuDevice.h"
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

    std::unique_ptr<GpuDevice> m_gpu;
    std::unique_ptr<Renderer> m_renderer;
};
