//
// Created by andyroiiid on 12/13/2022.
//

#include "VulkanBase.h"

#include <algorithm>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

#include "Debug.h"

VulkanBase::VulkanBase(GLFWwindow *window, bool vsync, size_t numBuffering)
        : VulkanDevice(window) {
    CreateImmediateContext();
    CreateSwapchain(vsync);
    CreateSwapchainImageViews();
    CreateDepthStencilImageAndViews();
    CreateBufferingObjects(numBuffering);
}

void VulkanBase::CreateImmediateContext() {
    m_immediateFence = CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
    m_immediateCommandPool = CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    m_immediateCommandBuffer = AllocateCommandBuffer(m_immediateCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

static VkExtent2D CalcSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
    if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max() &&
        capabilities.currentExtent.height == std::numeric_limits<uint32_t>::max()) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D extent = {
                std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
        return extent;
    }
    return capabilities.currentExtent;
}

void VulkanBase::CreateSwapchain(bool vsync) {
    VkSurfaceCapabilitiesKHR capabilities;
    DebugCheckCriticalVk(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities),
            "Failed to get Vulkan physical device surface capabilities."
    );
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    m_swapchainExtent = CalcSwapchainExtent(capabilities, m_window);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_surfaceFormat.format;
    createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    createInfo.imageExtent = m_swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    uint32_t queueFamilyIndices[] = {
            m_graphicsQueueFamilyIndex,
            m_presentQueueFamilyIndex
    };
    if (m_graphicsQueueFamilyIndex != m_presentQueueFamilyIndex) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = vsync ? VK_PRESENT_MODE_FIFO_KHR : m_presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = m_swapchain;
    DebugCheckCriticalVk(
            vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain),
            "Failed to create Vulkan swapchain."
    );

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
}

void VulkanBase::CreateSwapchainImageViews() {
    size_t numImages = m_swapchainImages.size();
    m_swapchainImageViews.resize(numImages);
    for (int i = 0; i < numImages; i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_surfaceFormat.format;
        VkImageSubresourceRange &subresourceRange = createInfo.subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        m_swapchainImageViews[i] = CreateImageView(createInfo);
    }
}

void VulkanBase::CreateDepthStencilImageAndViews() {
    size_t numImages = m_swapchainImages.size();
    m_depthStencilImages.resize(numImages);
    m_depthStencilImageViews.resize(numImages);
    for (int i = 0; i < numImages; i++) {
        m_depthStencilImages[i] = CreateImage2D(
                m_depthStencilFormat,
                m_swapchainExtent,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                0,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        );
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_depthStencilImages[i].Get();
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m_depthStencilFormat;
        VkImageSubresourceRange &depthImageViewSubresourceRange = imageViewCreateInfo.subresourceRange;
        depthImageViewSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthImageViewSubresourceRange.baseMipLevel = 0;
        depthImageViewSubresourceRange.levelCount = 1;
        depthImageViewSubresourceRange.baseArrayLayer = 0;
        depthImageViewSubresourceRange.layerCount = 1;
        m_depthStencilImageViews[i] = CreateImageView(imageViewCreateInfo);
    }
}

void VulkanBase::CreateBufferingObjects(size_t numBuffering) {
    m_bufferingObjects.resize(numBuffering);
    for (BufferingObjects &bufferingObjects: m_bufferingObjects) {
        bufferingObjects.RenderFence = CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
        bufferingObjects.PresentSemaphore = CreateSemaphore();
        bufferingObjects.RenderSemaphore = CreateSemaphore();

        bufferingObjects.CommandPool = CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        bufferingObjects.CommandBuffer = AllocateCommandBuffer(bufferingObjects.CommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }
}

VulkanBase::~VulkanBase() {
    WaitIdle();

    for (const BufferingObjects &bufferingObjects: m_bufferingObjects) {
        FreeCommandBuffer(bufferingObjects.CommandPool, bufferingObjects.CommandBuffer);
        DestroyCommandPool(bufferingObjects.CommandPool);

        DestroySemaphore(bufferingObjects.PresentSemaphore);
        DestroySemaphore(bufferingObjects.RenderSemaphore);
        DestroyFence(bufferingObjects.RenderFence);
    }

    for (auto &depthStencilImageView: m_depthStencilImageViews) {
        DestroyImageView(depthStencilImageView);
    }
    for (auto &depthStencilImage: m_depthStencilImages) {
        depthStencilImage = {};
    }
    for (auto &swapchainImageView: m_swapchainImageViews) {
        DestroyImageView(swapchainImageView);
    }
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    FreeCommandBuffer(m_immediateCommandPool, m_immediateCommandBuffer);
    DestroyCommandPool(m_immediateCommandPool);
    DestroyFence(m_immediateFence);
}

void VulkanBase::ImGuiInit(VkRenderPass renderPass) {
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplGlfw_InitForVulkan(m_window, true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_instance;
    initInfo.PhysicalDevice = m_physicalDevice;
    initInfo.Device = m_device;
    initInfo.QueueFamily = m_graphicsQueueFamilyIndex;
    initInfo.Queue = m_graphicsQueue;
    initInfo.DescriptorPool = m_descriptorPool;
    initInfo.MinImageCount = GetNumBuffering();
    initInfo.ImageCount = GetNumBuffering();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.CheckVkResultFn = [](VkResult result) {
        DebugCheckVk(result, "Vulkan error in ImGui: {}", result);
    };
    ImGui_ImplVulkan_Init(&initInfo, renderPass);

    ImmediateSubmit([](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    m_imguiEnabled = true;
}

void VulkanBase::ImGuiShutdown() {
    m_imguiEnabled = false;

    WaitIdle();
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void VulkanBase::ImGuiNewFrame() { // NOLINT(readability-make-member-function-const)
    if (!m_imguiEnabled) return;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VulkanBase::ImGuiRender(VkCommandBuffer commandBuffer) { // NOLINT(readability-make-member-function-const)
    if (!m_imguiEnabled) return;

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

VulkanBase::BeginFrameInfo VulkanBase::BeginFrame() {
    BufferingObjects &bufferingObjects = m_bufferingObjects[m_currentBufferingIndex];

    WaitForFence(bufferingObjects.RenderFence);
    ResetFence(bufferingObjects.RenderFence);

    DebugCheckCriticalVk(
            vkAcquireNextImageKHR(m_device, m_swapchain, 1'000'000'000, bufferingObjects.PresentSemaphore, nullptr, &m_currentSwapchainImageIndex),
            "Failed to acquire next Vulkan swapchain image."
    );

    ResetCommandBuffer(bufferingObjects.CommandBuffer);
    BeginCommandBuffer(bufferingObjects.CommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    return {m_currentSwapchainImageIndex, m_currentBufferingIndex, bufferingObjects.CommandBuffer};
}

void VulkanBase::EndFrame() {
    BufferingObjects &bufferingObjects = m_bufferingObjects[m_currentBufferingIndex];

    EndCommandBuffer(bufferingObjects.CommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &bufferingObjects.PresentSemaphore;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &bufferingObjects.CommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &bufferingObjects.RenderSemaphore;
    SubmitToGraphicsQueue(submitInfo, bufferingObjects.RenderFence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &bufferingObjects.RenderSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentSwapchainImageIndex;
    DebugCheckCriticalVk(
            vkQueuePresentKHR(m_graphicsQueue, &presentInfo),
            "Failed to present Vulkan swapchain image."
    );

    m_currentFrameCount++;
    m_currentBufferingIndex = m_currentFrameCount % m_bufferingObjects.size();
}
