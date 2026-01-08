#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanContext;

class VulkanSwapchain {
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain();

    // Non-copyable
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    // Movable
    VulkanSwapchain(VulkanSwapchain&& other) noexcept;
    VulkanSwapchain& operator=(VulkanSwapchain&& other) noexcept;

    bool create(VulkanContext& context, vk::Extent2D extent);
    void cleanup(VulkanContext& context);

    bool isInitialized() const { return m_initialized; }
    vk::SwapchainKHR getSwapchain() const { return m_swapchain; }
    const std::vector<vk::ImageView>& getImageViews() const { return m_imageViews; }
    const std::vector<vk::Framebuffer>& getFramebuffers() const { return m_framebuffers; }
    vk::RenderPass getRenderPass() const { return m_renderPass; }
    vk::Extent2D getExtent() const { return m_extent; }
    vk::Format getImageFormat() const { return m_imageFormat; }

    uint32_t acquireNextImage();

private:
    bool createImageViews(VulkanContext& context);
    bool createDepthBuffer(VulkanContext& context);
    bool createRenderPass(VulkanContext& context);
    bool createFramebuffers(VulkanContext& context);

    vk::SwapchainKHR m_swapchain;
    std::vector<vk::Image> m_images;
    std::vector<vk::ImageView> m_imageViews;
    std::vector<vk::Framebuffer> m_framebuffers;
    vk::RenderPass m_renderPass;
    vk::Image m_depthImage;
    vk::DeviceMemory m_depthMemory;
    vk::ImageView m_depthImageView;

    vk::Extent2D m_extent;
    vk::Format m_imageFormat;
    vk::Format m_depthFormat;

    bool m_initialized = false;
};
