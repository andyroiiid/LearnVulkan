//
// Created by andyroiiid on 12/11/2022.
//

#include "Window.h"

#include <set>

#include "Debug.h"

Window::Window() {
    DebugCheckCritical(glfwInit() == GLFW_TRUE, "Failed to init GLFW.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(800, 600, "Learn Vulkan", nullptr, nullptr);
    DebugCheckCritical(m_window != nullptr, "Failed to create GLFW window.");

    InitVulkan();
}

Window::~Window() {
    DestroyVulkan();

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void Window::InitVulkan() {
    CreateInstance();
    CreateDebugMessenger();
    CreateSurface();
    SelectPhysicalDeviceAndGraphicsQueueFamilyIndex();
    CreateDevice();
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

void Window::CreateInstance() {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Learn Vulkan";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Learn Vulkan";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

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
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
) {
    DebugLog("Vulkan debug message: {}", pCallbackData->pMessage);
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

void Window::CreateDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    createInfo.pfnUserCallback = VulkanDebugCallback;
    createInfo.pUserData = nullptr;

    DebugCheckCriticalVk(
            vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger),
            "Failed to create Vulkan debug messenger."
    );
}

void Window::CreateSurface() {
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

void Window::SelectPhysicalDeviceAndGraphicsQueueFamilyIndex() {
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

        DebugLog(
                "Found physical device {} with graphics queue family {} and present queue family {}.",
                deviceProperties.deviceName,
                graphicsQueueFamilyIndex,
                presentQueueFamilyIndex
        );
        m_physicalDevice = device;
        m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
        m_presentQueueFamilyIndex = presentQueueFamilyIndex;
        break;
    }
    DebugCheckCritical(
            m_physicalDevice != VK_NULL_HANDLE && m_graphicsQueueFamilyIndex >= 0,
            "Failed to find a suitable Vulkan physical device."
    );
}

static std::vector<const char *> GetEnabledDeviceExtensions() {
    return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

void Window::CreateDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = {m_graphicsQueueFamilyIndex, m_presentQueueFamilyIndex};
    float queuePriority = 1.0f;
    for (int queueFamilyIndex: queueFamilyIndices) {
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

void Window::DestroyVulkan() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void Window::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}
