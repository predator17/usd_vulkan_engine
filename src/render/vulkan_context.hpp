#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>

// Forward declaration for GLFWwindow
struct GLFWwindow;
#include "vulkan_swapchain.hpp"

struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    // Non-copyable
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // Movable
    VulkanContext(VulkanContext&& other) noexcept;
    VulkanContext& operator=(VulkanContext&& other) noexcept;

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

    VulkanSwapchain& getSwapchain() { return m_swapchain; }
    const VulkanSwapchain& getSwapchain() const { return m_swapchain; }

    vk::CommandPool getCommandPool() const { return m_commandPool; }

    bool getFramebufferResized() const { return m_framebufferResized; }
    void setFramebufferResized(bool resized) { m_framebufferResized = resized; }

    float getElapsedTime() const { return m_elapsedTime; }
    void setElapsedTime(float time) { m_elapsedTime = time; }

    /**
     * @brief Find memory type with given properties
     * @param typeFilter Bit mask of memory types to consider
     * @param properties Required memory properties
     * @return Memory type index, or UINT32_MAX if not found
     */
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

private:
    // Required device extensions
    static const std::vector<const char*> s_deviceExtensions;

    bool createInstance(bool enableValidationLayers);
    bool createSurface(GLFWwindow* window);
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createCommandPool();
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

    vk::Instance m_instance;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_device;
    vk::SurfaceKHR m_surface;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    vk::CommandPool m_commandPool;

    VulkanSwapchain m_swapchain;

    VulkanQueueFamilyIndices m_queueFamilyIndices;
    bool m_initialized = false;
    bool m_framebufferResized = false;
    float m_elapsedTime = 0.0f;

#ifdef USD_VULKAN_ENGINE_VALIDATION
    vk::DebugUtilsMessengerEXT m_debugMessenger;
#endif
};
