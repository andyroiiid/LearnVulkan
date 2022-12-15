//
// Created by andyroiiid on 12/12/2022.
//

#include "VulkanDevice.h"

#include <set>
#include <algorithm>

#include "Debug.h"

VulkanDevice::VulkanDevice(GLFWwindow *window) {
    m_window = window;
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface();
    SelectPhysicalDeviceAndQueueFamilyIndices();
    CreateDevice();
    CreateAllocator();
    CreateDescriptorPool();
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

void VulkanDevice::CreateInstance() {
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

void VulkanDevice::CreateDebugMessenger() {
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

void VulkanDevice::CreateSurface() {
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
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
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

void VulkanDevice::SelectPhysicalDeviceAndQueueFamilyIndices() {
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

void VulkanDevice::CreateDevice() {
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
    deviceFeatures.fillModeNonSolid = VK_TRUE;

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

void VulkanDevice::CreateAllocator() {
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

void VulkanDevice::CreateDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes{
            {VK_DESCRIPTOR_TYPE_SAMPLER,                1024},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024}
    };

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = 1024;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();

    DebugCheckCriticalVk(
            vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorPool),
            "Failed to create Vulkan descriptor pool."
    );
}

VulkanDevice::~VulkanDevice() {
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

VkRenderPass VulkanDevice::CreateRenderPass(const VkRenderPassCreateInfo &createInfo) {
    VkRenderPass renderPass = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass),
            "Failed to create Vulkan render pass."
    );
    return renderPass;
}

VkFramebuffer VulkanDevice::CreateFramebuffer(const VkFramebufferCreateInfo &createInfo) {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateFramebuffer(m_device, &createInfo, nullptr, &framebuffer),
            "Failed to create Vulkan framebuffer."
    );
    return framebuffer;
}

VkShaderModule VulkanDevice::CreateShaderModule(const VkShaderModuleCreateInfo &createInfo) {
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule),
            "Failed to create Vulkan shader module."
    );
    return shaderModule;
}

VkPipelineLayout VulkanDevice::CreatePipelineLayout(const VkPipelineLayoutCreateInfo &createInfo) {
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreatePipelineLayout(m_device, &createInfo, nullptr, &pipelineLayout),
            "Failed to create Vulkan pipeline layout."
    );
    return pipelineLayout;
}

VkPipeline VulkanDevice::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo &createInfo) {
    VkPipeline pipeline = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline),
            "Failed to create Vulkan pipeline."
    );
    return pipeline;
}

VkImageView VulkanDevice::CreateImageView(const VkImageViewCreateInfo &createInfo) {
    VkImageView imageView = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateImageView(m_device, &createInfo, nullptr, &imageView),
            "Failed to create Vulkan image view."
    );
    return imageView;
}

VkDescriptorSetLayout VulkanDevice::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo &createInfo) {
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    DebugCheckCriticalVk(
            vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &descriptorSetLayout),
            "Failed to create Vulkan descriptor set layout."
    );
    return descriptorSetLayout;
}

VkDescriptorSet VulkanDevice::AllocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout) {
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &descriptorSetLayout;
    DebugCheckCriticalVk(
            vkAllocateDescriptorSets(m_device, &allocateInfo, &descriptorSet),
            "Failed to allocate Vulkan descriptor set."
    );
    return descriptorSet;
}

void VulkanDevice::FreeDescriptorSet(VkDescriptorSet descriptorSet) {
    DebugCheckCriticalVk(
            vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &descriptorSet),
            "Failed to free Vulkan descriptor set."
    );
}
