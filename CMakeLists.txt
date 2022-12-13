cmake_minimum_required(VERSION 3.21)
project(LearnVulkan)

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(spdlog EXCLUDE_FROM_ALL)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

add_subdirectory(glm EXCLUDE_FROM_ALL)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(LearnVulkan
        main.cpp
        Debug.h
        Window.cpp Window.h
        GpuDevice.cpp GpuDevice.h
        Renderer.cpp Renderer.h
        )

target_compile_definitions(LearnVulkan PUBLIC GLFW_INCLUDE_VULKAN)

target_link_libraries(LearnVulkan PUBLIC spdlog glfw Vulkan::Vulkan glm)
