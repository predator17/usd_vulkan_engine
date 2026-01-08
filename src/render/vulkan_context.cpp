#include "vulkan_context.hpp"
#include "vulkan_swapchain.hpp"
#include "core/logger.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <set>
#include <cstring>

// Required device extensions for swapchain support
const std::vector<const char*> VulkanContext::s_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext() {
    shutdown();
}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
    : m_instance(std::exchange(other.m_instance, vk::Instance()))
    , m_physicalDevice(std::exchange(other.m_physicalDevice, vk::PhysicalDevice()))
    , m_device(std::exchange(other.m_device, vk::Device()))
    , m_surface(std::exchange(other.m_surface, vk::SurfaceKHR()))
    , m_graphicsQueue(std::exchange(other.m_graphicsQueue, vk::Queue()))
    , m_presentQueue(std::exchange(other.m_presentQueue, vk::Queue()))
    , m_commandPool(std::exchange(other.m_commandPool, vk::CommandPool()))
    , m_swapchain(std::move(other.m_swapchain))
    , m_queueFamilyIndices(other.m_queueFamilyIndices)
    , m_initialized(other.m_initialized)
    , m_framebufferResized(other.m_framebufferResized)
    , m_elapsedTime(other.m_elapsedTime)
{
    other.m_queueFamilyIndices = VulkanQueueFamilyIndices();
    other.m_initialized = false;
    other.m_framebufferResized = false;
    other.m_elapsedTime = 0.0f;
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept {
    if (this != &other) {
        shutdown();

        m_instance = std::exchange(other.m_instance, vk::Instance());
        m_physicalDevice = std::exchange(other.m_physicalDevice, vk::PhysicalDevice());
        m_device = std::exchange(other.m_device, vk::Device());
        m_surface = std::exchange(other.m_surface, vk::SurfaceKHR());
        m_graphicsQueue = std::exchange(other.m_graphicsQueue, vk::Queue());
        m_presentQueue = std::exchange(other.m_presentQueue, vk::Queue());
        m_commandPool = std::exchange(other.m_commandPool, vk::CommandPool());
        m_swapchain = std::move(other.m_swapchain);
        m_queueFamilyIndices = other.m_queueFamilyIndices;
        m_initialized = other.m_initialized;
        m_framebufferResized = other.m_framebufferResized;
        m_elapsedTime = other.m_elapsedTime;

        other.m_queueFamilyIndices = VulkanQueueFamilyIndices();
        other.m_initialized = false;
        other.m_framebufferResized = false;
        other.m_elapsedTime = 0.0f;
    }
    return *this;
}

bool VulkanContext::initialize(GLFWwindow* window, bool enableValidationLayers) {
    if (m_initialized) {
        return false;
    }

    if (!window) {
        return false;
    }

    if (!createInstance(enableValidationLayers)) {
        return false;
    }

    if (!createSurface(window)) {
        shutdown();
        return false;
    }

    if (!pickPhysicalDevice()) {
        shutdown();
        return false;
    }

    if (!createLogicalDevice()) {
        shutdown();
        return false;
    }

    if (!createCommandPool()) {
        shutdown();
        return false;
    }

    // Get window size for swapchain
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    vk::Extent2D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    // Create swapchain
    if (!m_swapchain.create(*this, extent)) {
        shutdown();
        return false;
    }

    m_initialized = true;
    return true;
}

void VulkanContext::shutdown() {
    // Destroy in reverse order of creation
    if (m_device) {
        if (m_commandPool) {
            m_device.destroyCommandPool(m_commandPool);
            m_commandPool = vk::CommandPool();
        }

        // Destroy swapchain first
        m_swapchain.cleanup(*this);

        m_device.destroy();
        m_device = vk::Device();
    }

    if (m_surface && m_instance) {
        m_instance.destroySurfaceKHR(m_surface);
        m_surface = vk::SurfaceKHR();
    }

#ifdef USD_VULKAN_ENGINE_VALIDATION
    if (m_debugMessenger && m_instance) {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
        m_debugMessenger = vk::DebugUtilsMessengerEXT();
    }
#endif

    if (m_instance) {
        m_instance.destroy();
        m_instance = vk::Instance();
    }

    // Reset queue family indices
    m_queueFamilyIndices.graphicsFamily.reset();
    m_queueFamilyIndices.presentFamily.reset();

    m_initialized = false;
}

bool VulkanContext::createInstance(bool enableValidationLayers) {
    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName("USD Vulkan Engine");
    appInfo.setApplicationVersion(VK_MAKE_VERSION(0, 1, 0));
    appInfo.setPEngineName("No Engine");
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setApiVersion(VK_API_VERSION_1_0);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo);
    createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
    createInfo.setPpEnabledExtensionNames(extensions.data());

    vk::Result result = vk::createInstance(&createInfo, nullptr, &m_instance);
    return result == vk::Result::eSuccess;
}

bool VulkanContext::createSurface(GLFWwindow* window) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_instance, window, nullptr, &surface) != VK_SUCCESS) {
        return false;
    }
    m_surface = surface;
    return true;
}

bool VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    m_instance.enumeratePhysicalDevices(&deviceCount, nullptr);

    if (deviceCount == 0) {
        return false;
    }

    std::vector<vk::PhysicalDevice> devices(deviceCount);
    m_instance.enumeratePhysicalDevices(&deviceCount, devices.data());

    for (const auto& device : devices) {
        m_physicalDevice = device;
        // Find queue families
        uint32_t queueFamilyCount = 0;
        device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

        std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
        device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                if (!m_queueFamilyIndices.graphicsFamily.has_value()) {
                    m_queueFamilyIndices.graphicsFamily = i;
                }
            }

            VkBool32 presentSupport = false;
            device.getSurfaceSupportKHR(i, m_surface, &presentSupport);

            if (presentSupport) {
                m_queueFamilyIndices.presentFamily = i;
            }

            if (m_queueFamilyIndices.isComplete()) {
                break;
            }
            i++;
        }

        // Check if device supports required extensions
        if (m_queueFamilyIndices.isComplete()) {
            if (checkDeviceExtensionSupport(device)) {
                break;
            }
        }
    }

    return m_queueFamilyIndices.isComplete();
}

bool VulkanContext::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    uint32_t extensionCount = 0;
    device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
    device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(s_deviceExtensions.begin(), s_deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanContext::createLogicalDevice() {
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.setQueueFamilyIndex(queueFamily);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    vk::DeviceCreateInfo createInfo{};
    createInfo.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()));
    createInfo.setPQueueCreateInfos(queueCreateInfos.data());
    createInfo.setPEnabledFeatures(&deviceFeatures);

    // Enable required device extensions (VK_KHR_swapchain)
    createInfo.setEnabledExtensionCount(static_cast<uint32_t>(s_deviceExtensions.size()));
    createInfo.setPpEnabledExtensionNames(s_deviceExtensions.data());

    vk::Result result = m_physicalDevice.createDevice(&createInfo, nullptr, &m_device);
    if (result != vk::Result::eSuccess) {
        return false;
    }

    m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    m_device.getQueue(m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);

    return true;
}

bool VulkanContext::createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setQueueFamilyIndex(m_queueFamilyIndices.graphicsFamily.value());
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    vk::Result result = m_device.createCommandPool(&poolInfo, nullptr, &m_commandPool);
    if (result != vk::Result::eSuccess) {
        return false;
    }
    return true;
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return UINT32_MAX;
}
