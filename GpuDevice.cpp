//
// Created by andyroiiid on 12/12/2022.
//

#include "GpuDevice.h"

#include <set>
#include <algorithm>

#include "Debug.h"

GpuDevice::GpuDevice(GLFWwindow *window) {
    m_window = window;
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface();
    SelectPhysicalDeviceAndQueueFamilyIndices();
    CreateDevice();
    CreateAllocator();

    CreateSwapchain();
    CreateSwapchainImageViews();
    CreateDepthStencilImageAndViews();

    CreateSyncPrimitives();
    CreateCommandPoolAndBuffer();
}

static std::vector<const char *> GetEnabledInstanceLayers() {
    return {
            "VK_LAYER_KHRONOS_validation"
    };
}

static std::vector<const char *> GetEnabledInstanceExtensions() {
    uint32_t numGlfwExtensions = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&numGlfwExtensions);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + numGlfwExtensions);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}

void GpuDevice::CreateInstance() {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Learn Vulkan";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Learn Vulkan";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    std::vector<const char *> enabledLayers = GetEnabledInstanceLayers();
    createInfo.enabledLayerCount = enabledLayers.size();
    createInfo.ppEnabledLayerNames = enabledLayers.data();
    std::vector<const char *> enabledExtensions = GetEnabledInstanceExtensions();
    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    DebugCheckCriticalVk(
            vkCreateInstance(&createInfo, nullptr, &m_instance),
            "Failed to create Vulkan instance."
    );
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        [[maybe_unused]] void *pUserData
) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        DebugError("Vulkan: {}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        DebugWarning("Vulkan: {}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        DebugInfo("Vulkan: {}", pCallbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        DebugVerbose("Vulkan: {}", pCallbackData->pMessage);
    } else {
        DebugError("Vulkan debug message with unknown severity level 0x{:X}: {}", messageSeverity, pCallbackData->pMessage);
    }
    return VK_FALSE;
}

static VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pMessenger
) {
    auto proc = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    DebugCheckCritical(proc != nullptr, "Failed to fetch vkCreateDebugUtilsMessengerEXT address.");
    return proc(instance, pCreateInfo, pAllocator, pMessenger);
}

static VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT messenger,
        const VkAllocationCallbacks *pAllocator
) {
    auto proc = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    DebugCheckCritical(proc != nullptr, "Failed to fetch vkDestroyDebugUtilsMessengerEXT address.");
    return proc(instance, messenger, pAllocator);
}

void GpuDevice::CreateDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = VulkanDebugCallback;
    createInfo.pUserData = nullptr;

    DebugCheckCriticalVk(
            vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger),
            "Failed to create Vulkan debug messenger."
    );
}

void GpuDevice::CreateSurface() {
    DebugCheckCriticalVk(
            glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface),
            "Failed to create Vulkan surface."
    );
}

static std::vector<VkPhysicalDevice> EnumeratePhysicalDevices(VkInstance instance) {
    uint32_t numDevices;
    vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
    std::vector<VkPhysicalDevice> devices(numDevices);
    vkEnumeratePhysicalDevices(instance, &numDevices, devices.data());
    return devices;
}

static std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilies(VkPhysicalDevice device) {
    uint32_t numQueueFamilies;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilies.data());
    return queueFamilies;
}

static int FindGraphicsQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> &queueFamilies) {
    for (int i = 0; i < queueFamilies.size(); i++) {
        const VkQueueFamilyProperties &queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return i;
        }
    }
    return -1;
}

static int FindPresentQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<VkQueueFamilyProperties> &queueFamilies) {
    for (int i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            return i;
        }
    }
    return -1;
}

static std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t numSurfaceFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numSurfaceFormats, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(numSurfaceFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numSurfaceFormats, surfaceFormats.data());
    return surfaceFormats;
}

static std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t numPresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numPresentModes, nullptr);
    std::vector<VkPresentModeKHR> presentModes(numPresentModes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numPresentModes, presentModes.data());
    return presentModes;
}

static VkSurfaceFormatKHR PickSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats) {
    for (const VkSurfaceFormatKHR &surfaceFormat: surfaceFormats) {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surfaceFormat;
        }
    }
    return surfaceFormats.front();
}

static VkPresentModeKHR PickPresentMode(const std::vector<VkPresentModeKHR> &presentModes) {
    for (const VkPresentModeKHR &presentMode: presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void GpuDevice::SelectPhysicalDeviceAndQueueFamilyIndices() {
    std::vector<VkPhysicalDevice> devices = EnumeratePhysicalDevices(m_instance);
    for (const VkPhysicalDevice &device: devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        std::vector<VkQueueFamilyProperties> queueFamilies = GetPhysicalDeviceQueueFamilies(device);

        int graphicsQueueFamilyIndex = FindGraphicsQueueFamilyIndex(queueFamilies);
        if (graphicsQueueFamilyIndex < 0) {
            continue;
        }

        int presentQueueFamilyIndex = FindPresentQueueFamilyIndex(device, m_surface, queueFamilies);
        if (presentQueueFamilyIndex < 0) {
            continue;
        }

        std::vector<VkSurfaceFormatKHR> surfaceFormats = GetSurfaceFormats(device, m_surface);
        if (surfaceFormats.empty()) {
            continue;
        }

        std::vector<VkPresentModeKHR> presentModes = GetPresentModes(device, m_surface);
        if (presentModes.empty()) {
            continue;
        }

        DebugInfo(
                "Found physical device {} with graphics queue family {} and present queue family {}.",
                deviceProperties.deviceName,
                graphicsQueueFamilyIndex,
                presentQueueFamilyIndex
        );
        m_physicalDevice = device;
        m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
        m_presentQueueFamilyIndex = presentQueueFamilyIndex;
        m_surfaceFormat = PickSurfaceFormat(surfaceFormats);
        m_presentMode = PickPresentMode(presentModes);
        break;
    }
    DebugCheckCritical(
            m_physicalDevice != VK_NULL_HANDLE,
            "Failed to find a suitable Vulkan physical device."
    );
}

static std::vector<const char *> GetEnabledDeviceExtensions() {
    return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

void GpuDevice::CreateDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> queueFamilyIndices = {m_graphicsQueueFamilyIndex, m_presentQueueFamilyIndex};
    float queuePriority = 1.0f;
    for (uint32_t queueFamilyIndex: queueFamilyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    std::vector<const char *> enabledExtensions = GetEnabledDeviceExtensions();
    deviceCreateInfo.enabledExtensionCount = enabledExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    DebugCheckCriticalVk(
            vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device),
            "Failed to create Vulkan logical device."
    );

    vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentQueueFamilyIndex, 0, &m_presentQueue);
}

void GpuDevice::CreateAllocator() {
    VmaAllocatorCreateInfo createInfo{};
    createInfo.physicalDevice = m_physicalDevice;
    createInfo.device = m_device;
    createInfo.instance = m_instance;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    DebugCheckCriticalVk(
            vmaCreateAllocator(&createInfo, &m_allocator),
            "Failed to create Vulkan Memory Allocator."
    );
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

void GpuDevice::CreateSwapchain() {
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
    createInfo.presentMode = m_presentMode;
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

void GpuDevice::CreateSwapchainImageViews() {
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

void GpuDevice::CreateDepthStencilImageAndViews() {
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
        imageViewCreateInfo.image = m_depthStencilImages[i].Image;
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

void GpuDevice::CreateSyncPrimitives() {
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    DebugCheckCriticalVk(
            vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_renderFence),
            "Failed to create Vulkan render fence."
    );

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    DebugCheckCriticalVk(
            vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_presentSemaphore),
            "Failed to create Vulkan present semaphore."
    );
    DebugCheckCriticalVk(
            vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderSemaphore),
            "Failed to create Vulkan render semaphore."
    );
}

void GpuDevice::CreateCommandPoolAndBuffer() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
    DebugCheckCriticalVk(
            vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool),
            "Failed to create Vulkan command pool."
    );

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    DebugCheckCriticalVk(
            vkAllocateCommandBuffers(m_device, &allocateInfo, &m_commandBuffer),
            "Failed to allocate Vulkan command buffer."
    );
}

GpuDevice::~GpuDevice() {
    DebugCheckCriticalVk(
            WaitIdle(),
            "Failed to wait for Vulkan device when trying to cleanup."
    );

    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroySemaphore(m_device, m_presentSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_renderSemaphore, nullptr);
    vkDestroyFence(m_device, m_renderFence, nullptr);

    for (auto &depthStencilImageView: m_depthStencilImageViews) {
        DestroyImageView(depthStencilImageView);
    }
    for (auto &depthStencilImage: m_depthStencilImages) {
        DestroyImage(depthStencilImage);
    }
    for (auto &swapchainImageView: m_swapchainImageViews) {
        DestroyImageView(swapchainImageView);
    }
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

VkRenderPass GpuDevice::CreateRenderPass(const VkRenderPassCreateInfo &createInfo) {
    VkRenderPass renderPass = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass),
            "Failed to create Vulkan render pass."
    );
    return renderPass;
}

VkFramebuffer GpuDevice::CreateFramebuffer(const VkFramebufferCreateInfo &createInfo) {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateFramebuffer(m_device, &createInfo, nullptr, &framebuffer),
            "Failed to create Vulkan framebuffer."
    );
    return framebuffer;
}

VkShaderModule GpuDevice::CreateShaderModule(const VkShaderModuleCreateInfo &createInfo) {
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule),
            "Failed to create Vulkan shader module."
    );
    return shaderModule;
}

VkPipelineLayout GpuDevice::CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo) {
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreatePipelineLayout(m_device, &createInfo, nullptr, &pipelineLayout),
            "Failed to create Vulkan pipeline layout."
    );
    return pipelineLayout;
}

VkPipeline GpuDevice::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo) {
    VkPipeline pipeline = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline),
            "Failed to create Vulkan pipeline."
    );
    return pipeline;
}

GpuBuffer GpuDevice::CreateBuffer(const VkBufferCreateInfo &bufferCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) {
    GpuBuffer buffer;
    DebugCheckCriticalVk(
            vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer.Buffer, &buffer.Allocation, nullptr),
            "Failed to create Vulkan buffer."
    );
    return buffer;
}

GpuImage GpuDevice::CreateImage(const VkImageCreateInfo &imageCreateInfo, const VmaAllocationCreateInfo &allocationCreateInfo) {
    GpuImage image;
    DebugCheckCriticalVk(
            vmaCreateImage(m_allocator, &imageCreateInfo, &allocationCreateInfo, &image.Image, &image.Allocation, nullptr),
            "Failed to create Vulkan image."
    );
    return image;
}

VkImageView GpuDevice::CreateImageView(const VkImageViewCreateInfo &createInfo) {
    VkImageView imageView = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateImageView(m_device, &createInfo, nullptr, &imageView),
            "Failed to create Vulkan image view."
    );
    return imageView;
}

std::tuple<uint32_t, VkCommandBuffer> GpuDevice::BeginFrame() {
    DebugCheckCriticalVk(
            vkWaitForFences(m_device, 1, &m_renderFence, true, 1'000'000'000),
            "Failed to wait for Vulkan render fence."
    );
    DebugCheckCriticalVk(
            vkResetFences(m_device, 1, &m_renderFence),
            "Failed to reset Vulkan render fence."
    );
    DebugCheckCriticalVk(
            vkAcquireNextImageKHR(m_device, m_swapchain, 1'000'000'000, m_presentSemaphore, nullptr, &m_currentSwapchainImageIndex),
            "Failed to acquire next Vulkan swapchain image."
    );

    DebugCheckCriticalVk(
            vkResetCommandBuffer(m_commandBuffer, 0),
            "Failed to reset Vulkan command buffer."
    );

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    DebugCheckCriticalVk(
            vkBeginCommandBuffer(m_commandBuffer, &commandBufferBeginInfo),
            "Failed to begin Vulkan command buffer."
    );

    return {m_currentSwapchainImageIndex, m_commandBuffer};
}

void GpuDevice::EndFrame() {
    DebugCheckCriticalVk(
            vkEndCommandBuffer(m_commandBuffer),
            "Failed to end Vulkan command buffer."
    );

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_presentSemaphore;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderSemaphore;
    DebugCheckCriticalVk(
            vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_renderFence),
            "Failed to submit Vulkan command buffer."
    );

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentSwapchainImageIndex;
    DebugCheckCriticalVk(
            vkQueuePresentKHR(m_graphicsQueue, &presentInfo),
            "Failed to present Vulkan swapchain image."
    );
}

void *GpuDevice::MapMemory(VmaAllocation allocation) {
    void *data = nullptr;
    DebugCheckCriticalVk(
            vmaMapMemory(m_allocator, allocation, &data),
            "Failed to map Vulkan memory."
    );
    return data;
}

void GpuDevice::UnmapMemory(VmaAllocation allocation) {
    vmaUnmapMemory(m_allocator, allocation);
}
