#include "vulkan_context.hpp"
#include "core/logger.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <set>
#include <cstring>

VulkanContext::~VulkanContext() {
    shutdown();
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

    m_initialized = true;
    return true;
}

void VulkanContext::shutdown() {
    if (m_device) {
        m_device.destroy();
    }
    if (m_surface) {
        m_instance.destroySurfaceKHR(m_surface);
    }
#ifdef USD_VULKAN_ENGINE_VALIDATION
    if (m_debugMessenger) {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
    }
#endif
    if (m_instance) {
        m_instance.destroy();
    }

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

        if (m_queueFamilyIndices.isComplete()) {
            break;
        }
    }

    return m_queueFamilyIndices.isComplete();
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

    vk::Result result = m_physicalDevice.createDevice(&createInfo, nullptr, &m_device);
    if (result != vk::Result::eSuccess) {
        return false;
    }

    m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    m_device.getQueue(m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);

    return true;
}
