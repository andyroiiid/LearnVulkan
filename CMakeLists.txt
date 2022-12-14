cmake_minimum_required(VERSION 3.21)
project(LearnVulkan)

set(BUILD_SHARED_LIBS OFF)

include(NoRttiAndExceptions.cmake)

set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

find_package(Vulkan REQUIRED)

add_subdirectory(glslang EXCLUDE_FROM_ALL)

add_subdirectory(VulkanMemoryAllocator EXCLUDE_FROM_ALL)

add_subdirectory(glm EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 17)

add_executable(LearnVulkan
        main.cpp
        Debug.h
        Window.cpp Window.h
        vk_mem_alloc.cpp
        VulkanDevice.cpp VulkanDevice.h
        ShaderCompiler.cpp ShaderCompiler.h
        VertexBase.cpp VertexBase.h
        MeshUtilities.cpp MeshUtilities.h
        Renderer.cpp Renderer.h)

target_compile_definitions(LearnVulkan PUBLIC GLFW_INCLUDE_VULKAN GLM_FORCE_LEFT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(LearnVulkan PUBLIC spdlog glfw Vulkan::Vulkan glslang SPIRV glslang-default-resource-limits VulkanMemoryAllocator glm)
