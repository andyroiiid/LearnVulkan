cmake_minimum_required(VERSION 3.2)
project(imgui CXX)

find_package(Vulkan REQUIRED)

add_library(imgui
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui_impl_vulkan.cpp
        imgui_impl_glfw.cpp)

target_include_directories(imgui PUBLIC include)

target_compile_definitions(imgui PUBLIC GLFW_INCLUDE_VULKAN)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)