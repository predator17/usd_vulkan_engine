#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>

// Forward declaration for GLFWwindow
struct GLFWwindow;

struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanContext {
public:
    VulkanContext() = default;
    ~VulkanContext();

    // Non-copyable
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    bool initialize(GLFWwindow* window, bool enableValidationLayers);
    void shutdown();

    bool isInitialized() const { return m_initialized; }

    vk::Instance getInstance() const { return m_instance; }
    vk::PhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    vk::Device getDevice() const { return m_device; }
    vk::SurfaceKHR getSurface() const { return m_surface; }
    vk::Queue getGraphicsQueue() const { return m_graphicsQueue; }
    vk::Queue getPresentQueue() const { return m_presentQueue; }
    VulkanQueueFamilyIndices getQueueFamilyIndices() const { return m_queueFamilyIndices; }

private:
    // Required device extensions
    static const std::vector<const char*> s_deviceExtensions;

    bool createInstance(bool enableValidationLayers);
    bool createSurface(GLFWwindow* window);
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

    vk::Instance m_instance;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_device;
    vk::SurfaceKHR m_surface;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;

    VulkanQueueFamilyIndices m_queueFamilyIndices;
    bool m_initialized = false;

#ifdef USD_VULKAN_ENGINE_VALIDATION
    vk::DebugUtilsMessengerEXT m_debugMessenger;
#endif
};
